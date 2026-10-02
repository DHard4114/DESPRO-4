"""
Unit test for Gateway Logic:
- Store-and-Forward Two-Phase Commit & Fair Scheduling
- Strict MQTT Command Parser Validation
- Strict LoRa Telemetry Payload Validation
"""

import struct
import math
import json

# ==========================================
# 1. KONTRAK DATA TELEMETRY (34 Bytes)
# ==========================================
# schema_version (B = uint8)
# node_code (8s = char[8])
# sequence_no (I = uint32)
# uptime_seconds (I = uint32)
# water_level_cm (f = float)
# ammonia_ppm (f = float)
# h2s_ppm (f = float)
# battery_voltage (f = float)
# sos_triggered (B = uint8)
PAYLOAD_FORMAT = "<B8sI I f f f f B"
assert struct.calcsize(PAYLOAD_FORMAT) == 34, f"Payload size must be 34, got {struct.calcsize(PAYLOAD_FORMAT)}"

def pack_payload(schema_ver=1, node_code="WC_01", seq=1, uptime=100, water=-1.0, nh3=-1.0, h2s=-1.0, batt=-1.0, sos=0):
    node_bytes = node_code.encode('ascii').ljust(8, b'\x00')[:8]
    return struct.pack(PAYLOAD_FORMAT, schema_ver, node_bytes, seq, uptime, water, nh3, h2s, batt, sos)

def unpack_payload(buf):
    if len(buf) != 34:
        return None
    unpacked = struct.unpack(PAYLOAD_FORMAT, buf)
    raw_node = unpacked[1]
    # Check null-termination and printable
    null_idx = raw_node.find(b'\x00')
    if null_idx == -1:
        clean_node = raw_node.decode('ascii', errors='ignore')
    else:
        clean_node = raw_node[:null_idx].decode('ascii', errors='ignore')

    return {
        "schema_version": unpacked[0],
        "node_code": clean_node,
        "sequence_no": unpacked[2],
        "uptime_seconds": unpacked[3],
        "water_level_cm": unpacked[4],
        "ammonia_ppm": unpacked[5],
        "h2s_ppm": unpacked[6],
        "battery_voltage": unpacked[7],
        "sos_triggered": unpacked[8]
    }

def validate_telemetry_payload(buf):
    if len(buf) != 34:
        return False, "Length != 34"
    p = unpack_payload(buf)
    if p["schema_version"] != 1:
        return False, "schema_version != 1"
    if p["sos_triggered"] > 1:
        return False, "sos_triggered > 1"
    if len(p["node_code"]) == 0 or len(p["node_code"]) > 7:
        return False, "node_code invalid length"
    for c in p["node_code"]:
        if not (c.isalnum() or c in ('_', '-')):
            return False, f"Illegal char in node_code: {c}"

    floats = [p["water_level_cm"], p["ammonia_ppm"], p["h2s_ppm"], p["battery_voltage"]]
    for f in floats:
        if math.isnan(f) or math.isinf(f):
            return False, "Float is NaN or Inf"

    # Bounds
    if p["water_level_cm"] != -1.0 and not (0.0 <= p["water_level_cm"] <= 1000.0):
        return False, "water_level_cm out of bounds"
    if p["ammonia_ppm"] != -1.0 and not (0.0 <= p["ammonia_ppm"] <= 1000.0):
        return False, "ammonia_ppm out of bounds"
    if p["h2s_ppm"] != -1.0 and not (0.0 <= p["h2s_ppm"] <= 1000.0):
        return False, "h2s_ppm out of bounds"
    if p["battery_voltage"] != -1.0 and not (0.0 <= p["battery_voltage"] <= 15.0):
        return False, "battery_voltage out of bounds"

    return True, "OK"

# ==========================================
# 2. STORE-AND-FORWARD TWO-PHASE COMMIT SIM
# ==========================================
class CircularFlashBuffer:
    def __init__(self, capacity=5):
        self.capacity = capacity
        self.slots = [None] * capacity
        self.head = 0
        self.tail = 0
        self.count = 0
        self.head_seq = 0
        self.dropped_count = 0

    def push(self, payload):
        slot = self.tail
        self.slots[slot] = payload
        if self.count == 0:
            self.head_seq = payload["sequence_no"]
        self.tail = (self.tail + 1) % self.capacity
        if self.count < self.capacity:
            self.count += 1
        else:
            # Overwrite oldest at head
            self.head = (self.head + 1) % self.capacity
            self.dropped_count += 1
            self.head_seq = self.slots[self.head]["sequence_no"]
        return slot

    def peek(self):
        if self.count == 0:
            return None, None
        slot = self.head
        data = self.slots[slot]
        token = {
            "slot": slot,
            "sequence_no": data["sequence_no"],
            "valid": True
        }
        return data, token

    def commit(self, token):
        if not token["valid"] or self.count == 0:
            return False
        if self.head == token["slot"] and self.head_seq == token["sequence_no"]:
            self.head = (self.head + 1) % self.capacity
            self.count -= 1
            if self.count > 0:
                self.head_seq = self.slots[self.head]["sequence_no"]
            return True
        else:
            # Overwritten by wrap-around
            return False

# ==========================================
# 3. FAIR SCHEDULING SIMULATION
# ==========================================
def simulate_fair_scheduling(ram_queue, flash_buffer, drain_ratio=2):
    processed = []
    fair_counter = 0

    while ram_queue or flash_buffer.count > 0:
        # Check if we should drain from flash
        if flash_buffer.count > 0 and (fair_counter >= drain_ratio or not ram_queue):
            data, token = flash_buffer.peek()
            # Simulate successful MQTT publish
            success = True
            if success:
                flash_buffer.commit(token)
                processed.append(f"FLASH_SEQ_{data['sequence_no']}")
                fair_counter = 0
                continue

        if ram_queue:
            item = ram_queue.pop(0)
            processed.append(f"RAM_SEQ_{item['sequence_no']}")
            fair_counter += 1
        else:
            fair_counter = drain_ratio

    return processed

# ==========================================
# 4. STRICT MQTT COMMAND VALIDATION
# ==========================================
def validate_mqtt_command(topic, payload_str, gw_id="GW_POSKO_01", enable_downlink=False):
    # Check topic pattern
    parts = topic.split('/')
    if len(parts) != 4 or parts[0] != "esos" or parts[1] != gw_id or parts[3] != "command":
        return False, "TOPIC_MISMATCH", None

    try:
        doc = json.loads(payload_str)
    except Exception as e:
        return False, "INVALID_JSON", None

    if not isinstance(doc, dict):
        return False, "PAYLOAD_NOT_OBJECT", None

    if "node_code" not in doc or "command_id" not in doc or "parameter" not in doc:
        return False, "MISSING_MANDATORY_FIELD", None

    node_code = doc["node_code"]
    cmd_id = doc["command_id"]
    param = doc["parameter"]

    if not isinstance(node_code, str) or len(node_code) == 0 or len(node_code) > 7:
        return False, "INVALID_NODE_CODE_LENGTH", None

    for c in node_code:
        if not (c.isalnum() or c in ('_', '-')):
            return False, "ILLEGAL_CHAR_IN_NODE_CODE", None

    if not isinstance(cmd_id, int) or isinstance(cmd_id, bool) or cmd_id < 1 or cmd_id > 4:
        return False, "INVALID_COMMAND_ID", None

    if not isinstance(param, int) or isinstance(param, bool) or param < 0 or param > 180:
        return False, "INVALID_PARAMETER", None

    if not enable_downlink:
        return True, "DOWNLINK_UNAVAILABLE", {"node_code": node_code, "command_id": cmd_id, "parameter": param}

    return True, "QUEUED_DOWNLINK", {"node_code": node_code, "command_id": cmd_id, "parameter": param}

# ==========================================
# RUN TEST SUITE
# ==========================================
def run_tests():
    print("=== TEST 1: TELEMETRY PAYLOAD VALIDATION ===")
    valid_buf = pack_payload(seq=101, water=45.5, nh3=12.0, h2s=3.4, batt=3.85, sos=0)
    ok, msg = validate_telemetry_payload(valid_buf)
    assert ok, f"Expected OK, got {msg}"
    print(" Valid payload: PASS")

    # Sentinel -1.0f check
    sentinel_buf = pack_payload(seq=102, water=-1.0, nh3=-1.0, h2s=-1.0, batt=-1.0, sos=0)
    ok, msg = validate_telemetry_payload(sentinel_buf)
    assert ok, f"Expected OK for sentinel -1.0f, got {msg}"
    print(" Sentinel -1.0f preservation: PASS")

    # NaN / Inf rejection
    nan_buf = pack_payload(seq=103, water=float('nan'))
    ok, msg = validate_telemetry_payload(nan_buf)
    assert not ok and "NaN" in msg, f"Expected NaN rejection, got {ok}, {msg}"
    print(" NaN rejection: PASS")

    # Bad schema version
    bad_ver = pack_payload(schema_ver=2)
    ok, msg = validate_telemetry_payload(bad_ver)
    assert not ok, "Expected schema rejection"
    print(" Schema version != 1 rejection: PASS")

    # Long node code (> 7 chars)
    long_node = pack_payload(node_code="NODE_WAY_TOO_LONG")
    ok, msg = validate_telemetry_payload(long_node)
    assert not ok, "Expected node code length rejection"
    print(" Overlength node_code rejection: PASS")

    print("\n=== TEST 2: TWO-PHASE COMMIT & OVERWRITE SAFETY ===")
    buf = CircularFlashBuffer(capacity=3)
    p1 = {"sequence_no": 1}
    p2 = {"sequence_no": 2}
    p3 = {"sequence_no": 3}

    buf.push(p1)
    buf.push(p2)
    buf.push(p3)
    assert buf.count == 3
    assert buf.head == 0
    assert buf.head_seq == 1

    # Peek 1
    data, token = buf.peek()
    assert data["sequence_no"] == 1
    assert token["slot"] == 0
    assert token["sequence_no"] == 1
    assert buf.count == 3  # not deleted yet!

    # Simulate wrap-around during network publish: 3 new packets push into buffer
    buf.push({"sequence_no": 4}) # overwrites slot 0 (p1)
    assert buf.head == 1
    assert buf.head_seq == 2
    assert buf.dropped_count == 1

    # Now attempt to commit token 1 (slot 0, seq 1)
    commit_ok = buf.commit(token)
    assert not commit_ok, "Commit of overwritten token must fail!"
    assert buf.head == 1, "Head must NOT double increment!"
    assert buf.count == 3, "Count must remain 3!"
    print(" Wrap-around race condition safety: PASS")

    # Peek current head (seq 2) and commit normally
    data2, token2 = buf.peek()
    assert data2["sequence_no"] == 2
    commit_ok2 = buf.commit(token2)
    assert commit_ok2, "Commit of valid token must succeed"
    assert buf.count == 2
    assert buf.head == 2
    assert buf.head_seq == 3
    print(" Normal two-phase commit: PASS")

    print("\n=== TEST 3: FAIR SCHEDULING (STARVATION PREVENTION) ===")
    flash = CircularFlashBuffer(capacity=5)
    flash.push({"sequence_no": 501})
    flash.push({"sequence_no": 502})
    flash.push({"sequence_no": 503})

    # Continuous RAM queue of 6 packets
    ram = [{"sequence_no": i} for i in range(1, 7)]

    order = simulate_fair_scheduling(ram, flash, drain_ratio=2)
    print(" Processing order:", order)
    # Expected: 2 RAM, 1 Flash, 2 RAM, 1 Flash, 2 RAM, 1 Flash
    assert order == [
        "RAM_SEQ_1", "RAM_SEQ_2", "FLASH_SEQ_501",
        "RAM_SEQ_3", "RAM_SEQ_4", "FLASH_SEQ_502",
        "RAM_SEQ_5", "RAM_SEQ_6", "FLASH_SEQ_503"
    ], f"Unexpected fair scheduling order: {order}"
    print(" Fair scheduling 2:1 ratio interleaving: PASS")

    print("\n=== TEST 4: STRICT MQTT COMMAND VALIDATION ===")
    # Valid command with downlink disabled
    ok, status, cmd = validate_mqtt_command("esos/GW_POSKO_01/WC_01/command",
                                            '{"node_code":"WC_01","command_id":1,"parameter":90}',
                                            enable_downlink=False)
    assert ok and status == "DOWNLINK_UNAVAILABLE"
    assert cmd["node_code"] == "WC_01" and cmd["command_id"] == 1 and cmd["parameter"] == 90
    print(" Valid command -> DOWNLINK_UNAVAILABLE: PASS")

    # Missing parameter
    ok, err, _ = validate_mqtt_command("esos/GW_POSKO_01/WC_01/command",
                                       '{"node_code":"WC_01","command_id":1}')
    assert not ok and err == "MISSING_MANDATORY_FIELD"
    print(" Missing mandatory field rejection: PASS")

    # Negative parameter
    ok, err, _ = validate_mqtt_command("esos/GW_POSKO_01/WC_01/command",
                                       '{"node_code":"WC_01","command_id":1,"parameter":-10}')
    assert not ok and err == "INVALID_PARAMETER"
    print(" Negative parameter rejection: PASS")

    # Parameter overflow (> 180)
    ok, err, _ = validate_mqtt_command("esos/GW_POSKO_01/WC_01/command",
                                       '{"node_code":"WC_01","command_id":1,"parameter":250}')
    assert not ok and err == "INVALID_PARAMETER"
    print(" Parameter overflow rejection: PASS")

    # Invalid command_id (> 4)
    ok, err, _ = validate_mqtt_command("esos/GW_POSKO_01/WC_01/command",
                                       '{"node_code":"WC_01","command_id":99,"parameter":0}')
    assert not ok and err == "INVALID_COMMAND_ID"
    print(" Invalid command_id rejection: PASS")

    # Illegal chars in node_code
    ok, err, _ = validate_mqtt_command("esos/GW_POSKO_01/WC_01/command",
                                       '{"node_code":"WC;01","command_id":1,"parameter":0}')
    assert not ok and err == "ILLEGAL_CHAR_IN_NODE_CODE"
    print(" Illegal char rejection: PASS")

    # Topic mismatch
    ok, err, _ = validate_mqtt_command("esos/OTHER_GW/WC_01/command",
                                       '{"node_code":"WC_01","command_id":1,"parameter":0}')
    assert not ok and err == "TOPIC_MISMATCH"
    print(" Topic mismatch rejection: PASS")

    print("\n=== TEST 5: TIME SYNC PARSING & BOUNDS ===")
    def parse_time_sync(payload_str):
        if not payload_str or len(payload_str) > 32:
            return False, "INVALID_LENGTH", 0
        if not payload_str.isdigit():
            return False, "NOT_ALL_DIGITS", 0
        val = int(payload_str)
        if val < 1700000000 or val >= 2500000000:
            return False, "OUT_OF_BOUNDS", val
        return True, "OK", val

    ok, msg, val = parse_time_sync("1727876543")
    assert ok and val == 1727876543
    print(" Valid server epoch: PASS")

    ok, msg, _ = parse_time_sync("1727abc543")
    assert not ok and msg == "NOT_ALL_DIGITS"
    print(" Non-digit rejection: PASS")

    ok, msg, _ = parse_time_sync("12345")
    assert not ok and msg == "OUT_OF_BOUNDS"
    print(" Out of bounds epoch rejection: PASS")

    ok, msg, _ = parse_time_sync("9" * 35)
    assert not ok and msg == "INVALID_LENGTH"
    print(" Overlength payload rejection: PASS")

    print("\n=== TEST 6: GATEWAY RECORD (38B) & JSON CONTRACT ===")
    # 34-byte TelemetryPayload + 4-byte gateway_timestamp (I) = 38 bytes
    GATEWAY_RECORD_FORMAT = "<B8sI I f f f f B I"
    assert struct.calcsize(GATEWAY_RECORD_FORMAT) == 38, f"Expected 38, got {struct.calcsize(GATEWAY_RECORD_FORMAT)}"

    # Pack record
    raw_payload = pack_payload(seq=12, uptime=450, water=30.0, nh3=1.2, h2s=0.5, batt=3.9, sos=0)
    gw_epoch = 1727879999
    raw_record = raw_payload + struct.pack("<I", gw_epoch)
    assert len(raw_record) == 38

    # Unpack record
    unpacked_rec = struct.unpack(GATEWAY_RECORD_FORMAT, raw_record)
    uptime_sec = unpacked_rec[3]
    epoch_rec = unpacked_rec[9]
    assert uptime_sec == 450, f"Uptime must be 450, got {uptime_sec}"
    assert epoch_rec == gw_epoch, f"Gateway epoch must be {gw_epoch}, got {epoch_rec}"
    print(" Separate uptime_seconds and gateway_timestamp: PASS")

    # JSON representation
    json_out = {
        "schema_version": unpacked_rec[0],
        "node_code": "WC_01",
        "sequence_no": unpacked_rec[2],
        "timestamp": epoch_rec,          # Unix epoch for backend
        "uptime_seconds": uptime_sec,    # True node uptime
        "water_level_cm": unpacked_rec[4],
        "ammonia_ppm": unpacked_rec[5],
        "h2s_ppm": unpacked_rec[6],
        "battery_voltage": unpacked_rec[7],
        "sos_triggered": unpacked_rec[8]
    }
    json_str = json.dumps(json_out)
    parsed_back = json.loads(json_str)
    assert parsed_back["timestamp"] == gw_epoch
    assert parsed_back["uptime_seconds"] == 450
    print(" JSON contract preservation (timestamp + uptime_seconds): PASS")

    print("\n>>> ALL UNIT TESTS PASSED SUCCESSFULLY! <<<")

if __name__ == "__main__":
    run_tests()


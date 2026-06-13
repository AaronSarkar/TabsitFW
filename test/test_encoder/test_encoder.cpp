#include <unity.h>
#include "Arduino.h"
#include "config/pins.h"
#include "input/encoder.h"

void setUp(void) {
    mock_reset_pins();
    resetEncoder();
}

void tearDown(void) {}

// --- getEncoderRawCount / resetEncoder ---

void test_initial_raw_count_is_zero(void) {
    TEST_ASSERT_EQUAL_INT(0, getEncoderRawCount());
}

void test_reset_encoder_clears_raw_count(void) {
    setEncoderRawCount(42);
    TEST_ASSERT_EQUAL_INT(42, getEncoderRawCount());
    resetEncoder();
    TEST_ASSERT_EQUAL_INT(0, getEncoderRawCount());
}

// --- setEncoderRawCount ---

void test_set_encoder_raw_count_positive(void) {
    setEncoderRawCount(10);
    TEST_ASSERT_EQUAL_INT(10, getEncoderRawCount());
}

void test_set_encoder_raw_count_negative(void) {
    setEncoderRawCount(-5);
    TEST_ASSERT_EQUAL_INT(-5, getEncoderRawCount());
}

void test_set_encoder_raw_count_zero(void) {
    setEncoderRawCount(99);
    setEncoderRawCount(0);
    TEST_ASSERT_EQUAL_INT(0, getEncoderRawCount());
}

// --- getEncoderCount (rawCount / ENCODER_RATIO) ---

void test_encoder_count_is_raw_divided_by_ratio(void) {
    setEncoderRawCount(10);
    TEST_ASSERT_EQUAL_INT(10 / getEncoderRatio(), getEncoderCount());
}

void test_encoder_count_truncates_for_odd_raw(void) {
    setEncoderRawCount(7);
    TEST_ASSERT_EQUAL_INT(3, getEncoderCount());
}

void test_encoder_count_zero_when_raw_is_zero(void) {
    setEncoderRawCount(0);
    TEST_ASSERT_EQUAL_INT(0, getEncoderCount());
}

void test_encoder_count_negative(void) {
    setEncoderRawCount(-6);
    TEST_ASSERT_EQUAL_INT(-3, getEncoderCount());
}

void test_encoder_count_negative_truncation(void) {
    setEncoderRawCount(-3);
    TEST_ASSERT_EQUAL_INT(-1, getEncoderCount());
}

// --- getEncoderRatio ---

void test_encoder_ratio_is_two(void) {
    TEST_ASSERT_EQUAL_INT(2, getEncoderRatio());
}

// --- getEncoderDirection ---

void test_initial_direction_is_none(void) {
    // After initEncoder, no update has been called
    initEncoder();
    TEST_ASSERT_EQUAL_INT(ENCODER_NONE, getEncoderDirection());
}

// --- updateEncoder: CW rotation ---
// CW: encoderAValue changes and digitalRead(ENCODER_B) != encoderAValue

void test_update_encoder_cw_increments_raw_count(void) {
    // Initial state: ENCODER_A = 0
    mock_set_pin(ENCODER_A, 0);
    initEncoder();

    // Simulate CW tick: A transitions 0->1, B != A (B=0)
    mock_set_pin(ENCODER_A, 1);
    mock_set_pin(ENCODER_B, 0);
    updateEncoder();

    TEST_ASSERT_EQUAL_INT(1, getEncoderRawCount());
    TEST_ASSERT_EQUAL_INT(ENCODER_CW, getEncoderDirection());
}

void test_update_encoder_multiple_cw_ticks(void) {
    mock_set_pin(ENCODER_A, 0);
    initEncoder();

    // Tick 1: A: 0->1, B=0 (CW)
    mock_set_pin(ENCODER_A, 1);
    mock_set_pin(ENCODER_B, 0);
    updateEncoder();

    // Tick 2: A: 1->0, B=1 (CW: B != A, B=1 != A=0)
    mock_set_pin(ENCODER_A, 0);
    mock_set_pin(ENCODER_B, 1);
    updateEncoder();

    // Tick 3: A: 0->1, B=0 (CW)
    mock_set_pin(ENCODER_A, 1);
    mock_set_pin(ENCODER_B, 0);
    updateEncoder();

    TEST_ASSERT_EQUAL_INT(3, getEncoderRawCount());
}

// --- updateEncoder: CCW rotation ---
// CCW: encoderAValue changes and digitalRead(ENCODER_B) == encoderAValue

void test_update_encoder_ccw_decrements_raw_count(void) {
    mock_set_pin(ENCODER_A, 0);
    initEncoder();

    // Simulate CCW tick: A transitions 0->1, B == A (B=1)
    mock_set_pin(ENCODER_A, 1);
    mock_set_pin(ENCODER_B, 1);
    updateEncoder();

    TEST_ASSERT_EQUAL_INT(-1, getEncoderRawCount());
    TEST_ASSERT_EQUAL_INT(ENCODER_CCW, getEncoderDirection());
}

void test_update_encoder_multiple_ccw_ticks(void) {
    mock_set_pin(ENCODER_A, 0);
    initEncoder();

    // Tick 1: A: 0->1, B=1 (CCW)
    mock_set_pin(ENCODER_A, 1);
    mock_set_pin(ENCODER_B, 1);
    updateEncoder();

    // Tick 2: A: 1->0, B=0 (CCW: B == A, B=0 == A=0)
    mock_set_pin(ENCODER_A, 0);
    mock_set_pin(ENCODER_B, 0);
    updateEncoder();

    // Tick 3: A: 0->1, B=1 (CCW)
    mock_set_pin(ENCODER_A, 1);
    mock_set_pin(ENCODER_B, 1);
    updateEncoder();

    TEST_ASSERT_EQUAL_INT(-3, getEncoderRawCount());
}

// --- updateEncoder: no change ---

void test_update_encoder_no_change_when_a_stable(void) {
    mock_set_pin(ENCODER_A, 0);
    initEncoder();

    // A hasn't changed, still 0
    mock_set_pin(ENCODER_A, 0);
    updateEncoder();

    TEST_ASSERT_EQUAL_INT(0, getEncoderRawCount());
    TEST_ASSERT_EQUAL_INT(ENCODER_NONE, getEncoderDirection());
}

// --- updateEncoder: direction resets each call ---

void test_direction_resets_to_none_on_no_movement(void) {
    mock_set_pin(ENCODER_A, 0);
    initEncoder();

    // CW tick
    mock_set_pin(ENCODER_A, 1);
    mock_set_pin(ENCODER_B, 0);
    updateEncoder();
    TEST_ASSERT_EQUAL_INT(ENCODER_CW, getEncoderDirection());

    // No change tick (A stays at 1)
    updateEncoder();
    TEST_ASSERT_EQUAL_INT(ENCODER_NONE, getEncoderDirection());
}

// --- updateEncoder: mixed CW and CCW ---

void test_update_encoder_mixed_directions(void) {
    mock_set_pin(ENCODER_A, 0);
    initEncoder();

    // CW tick: A: 0->1, B=0
    mock_set_pin(ENCODER_A, 1);
    mock_set_pin(ENCODER_B, 0);
    updateEncoder();
    TEST_ASSERT_EQUAL_INT(1, getEncoderRawCount());

    // CCW tick: A: 1->0, B=0 (B == A means CCW)
    mock_set_pin(ENCODER_A, 0);
    mock_set_pin(ENCODER_B, 0);
    updateEncoder();
    TEST_ASSERT_EQUAL_INT(0, getEncoderRawCount());

    // CCW tick: A: 0->1, B=1
    mock_set_pin(ENCODER_A, 1);
    mock_set_pin(ENCODER_B, 1);
    updateEncoder();
    TEST_ASSERT_EQUAL_INT(-1, getEncoderRawCount());
}

// --- encoder count reflects updates ---

void test_encoder_count_after_cw_ticks(void) {
    mock_set_pin(ENCODER_A, 0);
    initEncoder();

    // 4 CW ticks
    for (int i = 0; i < 4; i++) {
        int nextA = (i % 2 == 0) ? 1 : 0;
        int nextB = (nextA == 1) ? 0 : 1; // B != A for CW
        mock_set_pin(ENCODER_A, nextA);
        mock_set_pin(ENCODER_B, nextB);
        updateEncoder();
    }

    TEST_ASSERT_EQUAL_INT(4, getEncoderRawCount());
    TEST_ASSERT_EQUAL_INT(2, getEncoderCount());
}

// --- initEncoder reads initial pin state ---

void test_init_encoder_reads_initial_a_state(void) {
    mock_set_pin(ENCODER_A, 1);
    initEncoder();

    // A is already 1, calling update with A=1 should be no-op
    mock_set_pin(ENCODER_A, 1);
    updateEncoder();
    TEST_ASSERT_EQUAL_INT(0, getEncoderRawCount());

    // Now A transitions 1->0 with B=1 (B == A=0? No, B=1 != A=0 → CW)
    mock_set_pin(ENCODER_A, 0);
    mock_set_pin(ENCODER_B, 1);
    updateEncoder();
    TEST_ASSERT_EQUAL_INT(1, getEncoderRawCount());
    TEST_ASSERT_EQUAL_INT(ENCODER_CW, getEncoderDirection());
}

// --- initEncoder sets pin modes ---

void test_init_encoder_sets_pin_modes(void) {
    initEncoder();
    TEST_ASSERT_EQUAL_INT(INPUT, mock_pin_modes[ENCODER_A]);
    TEST_ASSERT_EQUAL_INT(INPUT, mock_pin_modes[ENCODER_B]);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // Raw count / reset
    RUN_TEST(test_initial_raw_count_is_zero);
    RUN_TEST(test_reset_encoder_clears_raw_count);

    // Set raw count
    RUN_TEST(test_set_encoder_raw_count_positive);
    RUN_TEST(test_set_encoder_raw_count_negative);
    RUN_TEST(test_set_encoder_raw_count_zero);

    // Encoder count (derived)
    RUN_TEST(test_encoder_count_is_raw_divided_by_ratio);
    RUN_TEST(test_encoder_count_truncates_for_odd_raw);
    RUN_TEST(test_encoder_count_zero_when_raw_is_zero);
    RUN_TEST(test_encoder_count_negative);
    RUN_TEST(test_encoder_count_negative_truncation);

    // Ratio
    RUN_TEST(test_encoder_ratio_is_two);

    // Direction
    RUN_TEST(test_initial_direction_is_none);

    // CW rotation
    RUN_TEST(test_update_encoder_cw_increments_raw_count);
    RUN_TEST(test_update_encoder_multiple_cw_ticks);

    // CCW rotation
    RUN_TEST(test_update_encoder_ccw_decrements_raw_count);
    RUN_TEST(test_update_encoder_multiple_ccw_ticks);

    // No change
    RUN_TEST(test_update_encoder_no_change_when_a_stable);

    // Direction resets
    RUN_TEST(test_direction_resets_to_none_on_no_movement);

    // Mixed
    RUN_TEST(test_update_encoder_mixed_directions);

    // Count after updates
    RUN_TEST(test_encoder_count_after_cw_ticks);

    // Init behavior
    RUN_TEST(test_init_encoder_reads_initial_a_state);
    RUN_TEST(test_init_encoder_sets_pin_modes);

    return UNITY_END();
}

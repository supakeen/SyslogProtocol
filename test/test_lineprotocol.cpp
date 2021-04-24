#include <Arduino.h>
#include <unity.h>

#include <LineProtocol.h>


void test_line_protocol_parse_0(void) {
    struct line_protocol lp;
    int result; 

    result = line_protocol_parse(lp, "measurement foo=bar");

    TEST_ASSERT_EQUAL(result, 0);
    TEST_ASSERT_EQUAL_STRING("measurement", lp.measurement.c_str());
}

void test_line_protocol_parse_1(void) {
    struct line_protocol lp;
    int result; 

    result = line_protocol_parse(lp, "test foo=bar");

    TEST_ASSERT_EQUAL(result, 0);
    TEST_ASSERT_EQUAL_STRING("test", lp.measurement.c_str());
}

void test_line_protocol_parse_2(void) {
    struct line_protocol lp;
    int result; 

    result = line_protocol_parse(lp, "test,bar=foo foo=bar");

    TEST_ASSERT_EQUAL(result, 0);
    TEST_ASSERT_EQUAL_STRING("test", lp.measurement.c_str());
    TEST_ASSERT_EQUAL_STRING("foo",  lp.tags["bar"].c_str());
    TEST_ASSERT_EQUAL_STRING("bar", lp.fields["foo"].c_str());
}

void test_line_protocol_parse_3(void) {
    struct line_protocol lp;
    int result; 

    result = line_protocol_parse(lp, "test,bar= foo foo=bar");

    TEST_ASSERT_EQUAL(result, -1);
}

void test_line_protocol_parse_4(void) {
    struct line_protocol lp;
    int result; 

    result = line_protocol_parse(lp, "test,bar=foo fo o=bar");

    TEST_ASSERT_EQUAL(result, -1);
}

void test_line_protocol_parse_5(void) {
    struct line_protocol lp;
    int result; 

    result = line_protocol_parse(lp, ",bar=foo foo=bar");

    TEST_ASSERT_EQUAL(result, -1);
}

void test_line_protocol_validate_0(void) {
    struct line_protocol lp;
    int result; 

    result = line_protocol_parse(lp, "test,bar=foo foo=bar");

    TEST_ASSERT_EQUAL(result, 0);

    TEST_ASSERT_EQUAL(0, line_protocol_validate(lp, { "bar" }, { "foo" }));
    TEST_ASSERT_EQUAL(1, line_protocol_validate(lp, { "foo" }, { "bar" }));
    TEST_ASSERT_EQUAL(0, line_protocol_validate(lp, { }, { }));
}


void setup() {
    UNITY_BEGIN();

    RUN_TEST(test_line_protocol_parse_0);
    RUN_TEST(test_line_protocol_parse_1);
    RUN_TEST(test_line_protocol_parse_2);
    RUN_TEST(test_line_protocol_parse_3);
    RUN_TEST(test_line_protocol_parse_4);
    RUN_TEST(test_line_protocol_parse_5);

    RUN_TEST(test_line_protocol_validate_0);
}

void loop() {
    UNITY_END();
}

int main() {
    setup();
    loop();

    return 0;
}

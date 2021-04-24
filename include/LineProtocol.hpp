/* This is a small header-only Arduino/C++ library to format or parse data in
 * the [InfluxDB Line Protocol](https://docs.influxdata.com/influxdb/v2.0/reference/syntax/line-protocol/).
 *
 * The InfluxDB Line Protocol looks like this:
 * ```
 * <measurement>[,<tag_key>=<tag_value>[,<tag_key>=<tag_value>]] <field_key>=<field_value>[,<field_key>=<field_value>] [<timestamp>]
 * ``` */

#pragma once

#include <stdlib.h>

#include <Arduino.h>

#include <map>
#include <list>

using namespace std;

namespace LineProtocol {
    struct line_protocol {
        String measurement;
        std::map<String, String> tags;
        std::map<String, String> fields;
        unsigned long long timestamp;
    };

    enum parse_state {
        PARSE_START = 0,
        PARSE_MEASUREMENT = 1,
        PARSE_TAGS = 2,
        PARSE_FIELDS = 3,
        PARSE_TIMESTAMP = 4,
        PARSE_END = 5,
        PARSE_ERR = 6,
    };

    enum subparse_state {
        SUBPARSE_START = 0,
        SUBPARSE_KEY = 1,
        SUBPARSE_VAL = 2,
        SUBPARSE_END = 3,
    };

    int line_protocol_format(struct line_protocol *lp, String& data) {
        return 0;
    }


    int line_protocol_validate(struct line_protocol &lp, std::list<String> tags, std::list<String> fields) {
        for(auto tag: tags) {
            if(!lp.tags.count(tag)) return 1;
        }

        for(auto field: fields) {
            if(!lp.fields.count(field)) return 1;
        }

        return 0;
    }

    int line_protocol_parse(struct line_protocol &lp, String data) {
        enum parse_state state = PARSE_START;
        enum subparse_state substate = SUBPARSE_START;

        bool in_escape = false;

        bool have_sep0 = false;
        bool have_sep1 = false;

        bool at_end = false;

        String key;
        String val;

        for(size_t i = 0; i < data.length(); i++) {
            /* Exit if we've encountered an error. The struct will be partially
             * filled! */

            if(state == PARSE_ERR) {
                break;
            }

            if(i+1 == data.length()) {
                at_end = true;
            }

            /* The following characters all have special meaning if they're not
             * being escaped. Most of them are either value separators or
             * section separators. */

            if(!in_escape) {
               if(data[i] == '\\') {
                   in_escape = true;
                   continue;
               }

               if(data[i] == ',') {
                   have_sep0 = true;
               } else {
                   have_sep0 = false;
               }

               if(data[i] == ' ') {
                   have_sep1 = true;
               } else {
                   have_sep1 = false;
               }
            }

            if(state == PARSE_START) {
                state = PARSE_MEASUREMENT;
            }

            /* The measurement is the initial data it needs to be non-zero
             * length before we can progress into the next step. */

            if(state == PARSE_MEASUREMENT) {

                /* If we encounter the , separator we move into tag parsing. */
                if(have_sep0) {

                    /* There needs to be at least one character of measurement
                     * before we can accept tags. */

                    if(!lp.measurement.length()) {
                        state = PARSE_ERR;
                        continue;
                    }

                    state = PARSE_TAGS;
                    continue;
                }

                /* If we encounter the ' ' separator we skip over tags into
                 * fields. */
                if(have_sep1) {

                    /* There needs to be at least one character of measurement
                     * before we can accept fields . */

                    if(!lp.measurement.length()) {
                        state = PARSE_ERR;
                        continue;
                    }


                    state = PARSE_FIELDS;
                    continue;
                }

                /* Add the character to the measurement name. */

                lp.measurement += data[i];
                continue;
            }

            if(state == PARSE_TAGS) {
                if(substate == SUBPARSE_START) {
                    substate = SUBPARSE_KEY;
                }

                if(substate == SUBPARSE_KEY) {
                    if(have_sep0 || have_sep1 || at_end) {
                        return -1;
                    }

                    if(data[i] == '=') {
                        substate = SUBPARSE_VAL;
                        continue;
                    }

                    key += data[i];
                    continue;
                }

                if(substate == SUBPARSE_VAL) {
                    if(have_sep0 || have_sep1 || at_end) {
                        if(at_end) {
                            val += data[i];
                        }

                        lp.tags[key] = val;

                        key.remove(0);
                        val.remove(0);

                        substate = SUBPARSE_KEY;

                        if(have_sep1) {
                            state = PARSE_FIELDS;
                        }

                        if(at_end) {
                            state = PARSE_END;
                        }

                        continue;
                    }

                    val += data[i];
                    continue;
                }
            }

            if(state == PARSE_FIELDS) {
                if(substate == SUBPARSE_KEY) {
                    if(have_sep0 || have_sep1 || at_end) {
                        return -1;
                    }

                    if(data[i] == '=') {
                        substate = SUBPARSE_VAL;
                        continue;
                    }

                    key += data[i];
                    continue;
                }

                if(substate == SUBPARSE_VAL) {
                    if(have_sep0 || have_sep1 || at_end) {
                        if(at_end) {
                            val += data[i];
                        }

                        lp.fields[key] = val;

                        key.remove(0);
                        val.remove(0);

                        substate = SUBPARSE_KEY;

                        if(have_sep1) {
                            state = PARSE_TIMESTAMP;
                        }

                        if(at_end) {
                            state = PARSE_END;
                        }

                        continue;
                    }

                    val += data[i];
                    continue;
                }
            }

            if(state == PARSE_TIMESTAMP) {
            }

            state = PARSE_END;
        }

        if(state == PARSE_ERR) {
            return -1;
        }

        return 0;
    }
}

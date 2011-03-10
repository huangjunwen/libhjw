#include <stdio.h>

/*
 * printf("%B", "abc"); -> <01100001, 01100010, 01100011>
 * printf("%*B", 3, "abcd"); -> <01100001, 01100010, 01100011>
 *
 *
 */

extern int extend_printf();

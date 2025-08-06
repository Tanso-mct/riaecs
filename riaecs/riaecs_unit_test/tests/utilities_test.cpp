#include "riaecs_unit_test/pch.h"

#include "riaecs/include/utilities.h"
#pragma comment(lib, "riaecs.lib");

TEST(Utilities, CreateMessage)
{
    std::string message = riaecs::CreateMessage({"Line 1", "Line 2", "Line 3"});
    EXPECT_EQ(message, "Line 1\nLine 2\nLine 3\n");

    message = riaecs::CreateMessage({"Line 1"}, __FILE__, __LINE__, __FUNCTION__);
    EXPECT_EQ
    (
        message, 
        "Line 1\n" + std::string(__FILE__) + " : " + std::to_string(11) + " : " + __FUNCTION__ + "\n"
    );
}
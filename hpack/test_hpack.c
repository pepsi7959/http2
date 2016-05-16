#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "testcase.h"
#include "hpack.h"


int test_dynamic_table_add(){
    char error[1024];
    error[0] = 0;
    DYNAMIC_TABLE *dt = (DYNAMIC_TABLE*)malloc(sizeof(DYNAMIC_TABLE));
    ASSERT( dt != NULL);
    memset(dt, 0 , sizeof(DYNAMIC_TABLE));
    
    ASSERT( dynamic_table_add(dt, ":path", "/insertSubscriber", error) == HPACK_RETURN_SUCCESS);
    ASSERT( dt->size == 1 );
    ASSERT( dt->header_fields != NULL );
    ASSERT( strcmp(dt->header_fields->name,":path") == 0 );
    ASSERT( strcmp(dt->header_fields->value, "/insertSubscriber") == 0 );
    
    ASSERT( dynamic_table_add(dt, ":scheme", "http", error) == HPACK_RETURN_SUCCESS);
    ASSERT( dt->size == 2 );
    ASSERT( dt->header_fields != NULL );
    ASSERT( strcmp(dt->header_fields->prev->name,":scheme") == 0 );
    ASSERT( strcmp(dt->header_fields->prev->value, "http") == 0 );
    
    return TEST_RESULT_SUCCESSED;
}

int test_dynamic_table_search(){
    char error[1024];
    error[0]    = 0;
    int isMatch = 0;
    int idx     = 0;
    DYNAMIC_TABLE *dt = (DYNAMIC_TABLE*)malloc(sizeof(DYNAMIC_TABLE));
    ASSERT( dt != NULL);
    memset(dt, 0 , sizeof(DYNAMIC_TABLE));
    ASSERT( dynamic_table_add(dt, ":path", "/insertSubscriber64", error) == HPACK_RETURN_SUCCESS);
    ASSERT( dynamic_table_add(dt, ":path", "/insertSubscriber63", error) == HPACK_RETURN_SUCCESS);
    ASSERT( dynamic_table_add(dt, ":path", "/insertSubscriber62", error) == HPACK_RETURN_SUCCESS);
    
    idx = dynamic_table_search(dt, ":path", "/insertSubscriber63", 0, &isMatch, error);
    ASSERT( idx == 63 );
    ASSERT( isMatch == 1);
    
    ASSERT( dynamic_table_add(dt, ":scheme", "ldap", error) == HPACK_RETURN_SUCCESS);
    idx = dynamic_table_search(dt, ":scheme", "ldap", 0, &isMatch, error);
    ASSERT( idx == 62);
    ASSERT( isMatch == 1);
    
    idx = dynamic_table_search(dt, ":path", "/insertSubscriber62", 0, &isMatch, error);
    ASSERT( idx == 63 );
    ASSERT( isMatch == 1);
    
    idx = dynamic_table_search(dt, ":scheme", "http", 0, &isMatch, error);
    ASSERT( idx == 6);
    ASSERT( isMatch == 1);
    

    
    return TEST_RESULT_SUCCESSED;
}

void test_all(){
    UNIT_TEST(test_dynamic_table_add());
    UNIT_TEST(test_dynamic_table_search());
}

int main(){
    test_all();
    return 0;
}

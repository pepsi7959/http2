#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "testcase.h"
#include "helloworld.pb-c.h"
#include "d21.pb-c.h"

void HEXDUMP(unsigned char *buff, int size){
	static char hex[] = {'0','1','2','3','4','5','6','7',
								'8','9','a','b','c','d','e','f'};
	int i = 0;
	for( i = 0;i < size; i++){
		unsigned char ch = buff[i];
		printf("(%u)%c", ch,hex[(ch>>4)]);
		printf("%c ", hex[(ch&0x0f)]);
	}
	printf("\n");
}

int test_helloworld(){
    Helloworld__HelloRequest *hw = malloc(sizeof(Helloworld__HelloRequest));//= HELLOWORLD__HELLO_REQUEST__INIT;
    helloworld__hello_request__init(hw);
    void *buf;
    unsigned len;
    hw->name = malloc(sizeof(char)*10);
    strcpy(hw->name, "pepsi1");
    len = helloworld__hello_request__get_packed_size(hw);
    printf("len : %d\n", len);
    buf = malloc(len);
    helloworld__hello_request__pack(hw, buf);
    HEXDUMP(buf, len);
    
    return TEST_RESULT_SUCCESSED;        
}

int test_Pb__Request(){
    Pb__Request *req = malloc(sizeof(Pb__Request));
    Pb__Request *decode_req = NULL;
    void *buf               = NULL;
    int len                 = 0;
    
    pb__request__init(req);
    
    req->has_id             = 1;
    req->id                 = 10;
    req->basedn             = calloc(1, sizeof(char)*1024);
    req->filter             = calloc(1, sizeof(char)*1024);
    req->dn                 = calloc(1, sizeof(char)*1024); 
    
    sprintf(req->basedn, "dc=C-NTDB");
    sprintf(req->filter, "objectclass=*");
    sprintf(req->dn, "uid=000000000000001,ds=subscriber,o=ais,dc=C-NTDB");
    
    req->has_scope          = 1;
    req->scope              = PB__SEARCH_SCOPE__BaseObject;
    req->has_recursive      = 1;
    req->recursive          = 1;
    
    req->entry              = NULL;
    
    Pb__Entry *entry        = malloc(sizeof(Pb__Entry));
    pb__entry__init(entry);
    entry->has_method       = 0;
    entry->method           = PB__ENTRY_METHOD__Add;
    entry->dn               = NULL;
    entry->dn               = malloc(sizeof(char)*1025);
    sprintf(entry->dn, "uid=000000000000001,ds=subscriber,o=ais,dc=C-NTDB");
    entry->n_attributes     = 0;
    entry->attributes       = NULL;
    
    Pb__EntryAttribute *attr_entry[1];
    char *values[2];
    
    attr_entry[0]           = calloc(1, sizeof(Pb__EntryAttribute));
    pb__entry_attribute__init(attr_entry[0]);

    attr_entry[0]->name     = malloc(sizeof(char)*128);
    sprintf(attr_entry[0]->name, "attr");
    
    attr_entry[0]->n_values = 2;
    values[0]               = malloc(sizeof(char)*128);
    values[1]               = malloc(sizeof(char)*128);

    sprintf(values[0], "values1");
    sprintf(values[1], "values2");
    attr_entry[0]->values   = values;
    
    entry->n_attributes     = 1;
    entry->attributes       = attr_entry;
    req->entry              = entry;
    
    
    len = pb__request__get_packed_size(req);
    buf = malloc(len);
    pb__request__pack(req, buf);
    HEXDUMP(buf, len);
    
    decode_req = pb__request__unpack(NULL, len, buf);
    
    ASSERT(decode_req != NULL);
    ASSERT(decode_req->id == 10);
    ASSERT(req->scope == PB__SEARCH_SCOPE__BaseObject);
    ASSERT(strcmp(req->basedn, "dc=C-NTDB") == 0);
    ASSERT(strcmp(req->filter, "objectclass=*") == 0);
    ASSERT(strcmp(req->dn, "uid=000000000000001,ds=subscriber,o=ais,dc=C-NTDB") == 0);
    ASSERT(req->recursive == 1);
    ASSERT(req->entry != NULL);
    ASSERT(req->entry->n_attributes == 1);
    ASSERT(req->entry->attributes != NULL);
    ASSERT( strcmp(req->entry->attributes[0]->name, "attr")== 0);
    ASSERT( req->entry->attributes[0]->n_values == 2);
    ASSERT( strcmp(req->entry->attributes[0]->values[0], "values1")== 0);
    ASSERT( strcmp(req->entry->attributes[0]->values[1], "values2")== 0);
    
    return TEST_RESULT_SUCCESSED;        
}

int test_Pb__Response(){
    Pb__Response *res       = malloc(sizeof(Pb__Response));
    Pb__Response *dec_res   = NULL;
    void *buf               = NULL;
    int len                 = 0;
    
    pb__response__init(res);
    
    res->has_id             = 1; 
    res->id                 = 0x10;
    res->has_resultcode     = 1;
    res->resultcode         = 0;
    res->matcheddn          = calloc(1, sizeof(char)*128);
    sprintf(res->matcheddn, "dc=C-NTDB");
    res->resultdescription  = NULL;
    res->n_referrals        = 0;
    res->referrals          = NULL;
    res->n_entries          = 1;
    
    Pb__EntryAttribute *entry[1];
    Pb__Entry          *en[1];
    char *values[2];
    entry[0]                = malloc(sizeof(Pb__Entry));
    entry[0]->name          = malloc(sizeof(char)*128);


    entry[0]->n_values      = 2;
    values[0]               = malloc(sizeof(char)*128);
    values[1]               = malloc(sizeof(char)*128);
    
    sprintf(entry[0]->name, "attribute");
    sprintf(values[0], "values1");
    sprintf(values[1], "values2");
    entry[0]->values        = values;
    
    en[0]->n_attributes     = 1;    
    en[0]->attributes       = entry;
    
    res->entries            = en;
    
    len = pb__response__get_packed_size(res);
    buf = malloc(len);
    pb__response__pack(res, buf);
    HEXDUMP(buf, len);
    
    dec_res = pb__response__unpack(NULL, len, buf);
    
    ASSERT(dec_res->has_id == 1); 
    ASSERT(dec_res->id == 0x10);
    ASSERT(dec_res->has_resultcode == 1);
    ASSERT(dec_res->resultcode == 0);
    ASSERT( strcmp(dec_res->matcheddn,"dc=C-NTDB") == 0 );
    ASSERT(dec_res->resultdescription == NULL);
    ASSERT(dec_res->n_entries == 0);
    ASSERT(dec_res->entries == NULL);
    ASSERT(dec_res->n_referrals == 0);
    ASSERT(dec_res->referrals == NULL);
    
    
    
    return TEST_RESULT_SUCCESSED;
}

void test_all(){
    UNIT_TEST(test_helloworld());
    UNIT_TEST(test_Pb__Request());
    UNIT_TEST(test_Pb__Response());
}

int main(){
    test_all();
    REPORT();
    return 0;
}

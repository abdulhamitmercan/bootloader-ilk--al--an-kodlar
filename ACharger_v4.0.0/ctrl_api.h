#ifndef FSM_API_H
#define FSM_API_H

#include <stdio.h>


typedef struct {
	unsigned int charging;
	unsigned int connector;
	unsigned int error;
} t_screenInputs;

t_screenInputs uiIn;

//-- ESP32 screen states
#define P1_IDLE			                 0
#define P2_PLUG_IN		               1
#define P3_SCAN_RFID	               2
#define P4_CHARGING		               3
#define P5_CHARGE_ENDED              4
#define P6_ERR         			         5


//-- INPUTS (N:No, Y:Yes) |CHARGING-CONNECTOR-ERROR|
#define NNN							 0 // no charging,  no connector,  no error
#define NNY							 1 // no charging,  no connector,  yes error
#define NYN							 2 // no charging,  yes connector, no error
#define NYY							 3 // no charging,  yes connector, yes error
#define YNN							 4 // yes charging, no connector,  no error
#define YNY							 5 // yes charging, no connector,  yes error
#define YYN							 6 // yes charging, yes connector, no error
#define YYY							 7 // yes charging, no connector,  yes error


//-- Screen Changes
#define NO_PAGE_CHANGE				 0

#define P2_to_P3					 1 // change from page2 to page3
#define P2_to_P4					 2 // change from page2 to page4
#define P2_to_P5					 3 // change from page2 to page5
#define P2_to_P6					 4 // change from page2 to page6

#define P3_to_P2					 5
#define P3_to_P4					 6
#define P3_to_P5					 7
#define P3_to_P6					 8

#define P4_to_P2					 9
#define P4_to_P3					 10
#define P4_to_P5					 11
#define P4_to_P6					 12

#define P5_to_P2					 13
#define P5_to_P3					 14
#define P5_to_P4					 15
#define P5_to_P6					 16

#define P6_to_P2					 17
#define P6_to_P3					 18
#define P6_to_P4					 19
#define P6_to_P5					 20


typedef struct {
    unsigned int Out; 
    unsigned int In; // 3 bit
    unsigned int current_state;
    unsigned int next_state;
} State;

State FSM;



void fsm_init(){
    FSM.Out &= 0x0000;
    FSM.In &= 0x0000;
    FSM.current_state = P2_PLUG_IN;
    FSM.next_state &= 0x0000;
}
void fsm_init2(){
    FSM.Out &= 0x0000;
    FSM.In &= 0x0000;
    FSM.current_state = P4_CHARGING	;
    FSM.next_state &= 0x0000;
}
void fsm_init_recovery(){
    FSM.Out = NO_PAGE_CHANGE;
    FSM.In = 0;
    FSM.current_state = P3_SCAN_RFID;
    FSM.next_state = P3_SCAN_RFID;
}
void fsm_update(unsigned int input, State* fsm){
	switch (fsm->current_state){
		//-- STATE P2_PLUG_IN ----
		case P2_PLUG_IN:
			switch (input){
				case NNN:
					fsm->next_state = P2_PLUG_IN;
					break;
				case YYN:
					fsm->next_state = P4_CHARGING; // tak çalıştır
					fsm->Out = P2_to_P4;
					break;
				case NYN:
					fsm->next_state = P3_SCAN_RFID;
					fsm->Out = P2_to_P3;
					break;
				//default:
					//fsm->next_state = P6_ERR;
					//fsm->Out = P2_to_P6;
					//break;
				}	
			break;
			
		//-- STATE P3_SCAN_RFID ----	
		case P3_SCAN_RFID:
			switch(input){
				case NNN:
					fsm->next_state = P2_PLUG_IN;
					fsm->Out = P3_to_P2;
					break;
				case NYN:
					fsm->next_state = P3_SCAN_RFID;
					break;
				case YYN:
					fsm->next_state = P4_CHARGING;
					fsm->Out = P3_to_P4;
					break;
				//default:
					//fsm->next_state = P6_ERR;
					//fsm->Out = P3_to_P6;
			}
			break;
			
		//-- STATE P4_CHARGING ----	
		case P4_CHARGING:
			switch(input){
				case NYN:
					fsm->next_state = P5_CHARGE_ENDED;
					fsm->Out = P4_to_P5;
					break;
				case YYN:
					fsm->next_state = P4_CHARGING;
					break;
        case NNN:
          fsm->next_state = P5_CHARGE_ENDED;
					fsm->Out = P4_to_P5;
        
				//default:
					//fsm->next_state = P6_ERR;
          //fsm->Out = P4_to_P6;
        
			}
			break;
			
		//-- STATE P5_CHARGE_ENDED ----	
		case P5_CHARGE_ENDED:
			switch(input){
				case NYN:
					fsm->next_state = P5_CHARGE_ENDED;
					break;
				case NNN:
					fsm->next_state = P2_PLUG_IN;
					fsm->Out = P5_to_P2;
					break;
				//default:
					//fsm->next_state = P6_ERR;
					//fsm->Out = P5_to_P6;
			}
			break;
			
		//-- STATE P6_ERR ----
		case P6_ERR:
			switch(input){
			case NNN:
				fsm->next_state = P2_PLUG_IN;
				fsm->Out = P6_to_P2;
				break;
			default:
				fsm->next_state = P6_ERR;
				break;

			}
			break;
	}
	
	fsm->current_state = fsm->next_state;
}


#endif 
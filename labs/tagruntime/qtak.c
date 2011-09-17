/* Generated from qtak.ss by the CHICKEN compiler
   http://www.call-with-current-continuation.org
   2011-09-16 14:46
   Version 4.6.5 
   macosx-unix-gnu-x86 [ manyargs dload ptables ]
   compiled 2011-03-23 on Arc-Loaner-Mac.local (Darwin)
   command line: qtak.ss -output-file qtak.c
   used units: library eval
*/

#include "chicken.h"

static C_PTABLE_ENTRY *create_ptable(void);
C_noret_decl(C_library_toplevel)
C_externimport void C_ccall C_library_toplevel(C_word c,C_word d,C_word k) C_noret;
C_noret_decl(C_eval_toplevel)
C_externimport void C_ccall C_eval_toplevel(C_word c,C_word d,C_word k) C_noret;

static C_TLS C_word lf[5];
static double C_possibly_force_alignment;
static C_char C_TLS li0[] C_aligned={C_lihdr(0,0,14),40,116,97,107,32,120,49,32,121,50,32,122,51,41,0,0};
static C_char C_TLS li1[] C_aligned={C_lihdr(0,0,10),40,116,111,112,108,101,118,101,108,41,0,0,0,0,0,0};


C_noret_decl(C_toplevel)
C_externexport void C_ccall C_toplevel(C_word c,C_word t0,C_word t1) C_noret;
C_noret_decl(f_16)
static void C_ccall f_16(C_word c,C_word t0,C_word t1) C_noret;
C_noret_decl(f_19)
static void C_ccall f_19(C_word c,C_word t0,C_word t1) C_noret;
C_noret_decl(f_74)
static void C_ccall f_74(C_word c,C_word t0,C_word t1) C_noret;
C_noret_decl(f_58)
static void C_ccall f_58(C_word c,C_word t0,C_word t1) C_noret;
C_noret_decl(f_61)
static void C_ccall f_61(C_word c,C_word t0,C_word t1) C_noret;
C_noret_decl(f_64)
static void C_ccall f_64(C_word c,C_word t0,C_word t1) C_noret;
C_noret_decl(f_70)
static void C_ccall f_70(C_word c,C_word t0,C_word t1) C_noret;
C_noret_decl(f_67)
static void C_ccall f_67(C_word c,C_word t0,C_word t1) C_noret;
C_noret_decl(f_21)
static void C_ccall f_21(C_word c,C_word t0,C_word t1,C_word t2,C_word t3,C_word t4) C_noret;
C_noret_decl(f_35)
static void C_ccall f_35(C_word c,C_word t0,C_word t1) C_noret;
C_noret_decl(f_39)
static void C_ccall f_39(C_word c,C_word t0,C_word t1) C_noret;
C_noret_decl(f_43)
static void C_ccall f_43(C_word c,C_word t0,C_word t1) C_noret;

C_noret_decl(tr5)
static void C_fcall tr5(C_proc5 k) C_regparm C_noret;
C_regparm static void C_fcall tr5(C_proc5 k){
C_word t4=C_pick(0);
C_word t3=C_pick(1);
C_word t2=C_pick(2);
C_word t1=C_pick(3);
C_word t0=C_pick(4);
C_adjust_stack(-5);
(k)(5,t0,t1,t2,t3,t4);}

C_noret_decl(tr2)
static void C_fcall tr2(C_proc2 k) C_regparm C_noret;
C_regparm static void C_fcall tr2(C_proc2 k){
C_word t1=C_pick(0);
C_word t0=C_pick(1);
C_adjust_stack(-2);
(k)(2,t0,t1);}

/* toplevel */
static C_TLS int toplevel_initialized=0;
C_main_entry_point
C_noret_decl(toplevel_trampoline)
static void C_fcall toplevel_trampoline(void *dummy) C_regparm C_noret;
C_regparm static void C_fcall toplevel_trampoline(void *dummy){
C_toplevel(2,C_SCHEME_UNDEFINED,C_restore);}

void C_ccall C_toplevel(C_word c,C_word t0,C_word t1){
C_word tmp;
C_word t2;
C_word t3;
C_word *a;
if(toplevel_initialized) C_kontinue(t1,C_SCHEME_UNDEFINED);
else C_toplevel_entry(C_text("toplevel"));
C_resize_stack(131072);
C_check_nursery_minimum(3);
if(!C_demand(3)){
C_save(t1);
C_reclaim((void*)toplevel_trampoline,NULL);}
toplevel_initialized=1;
if(!C_demand_2(50)){
C_save(t1);
C_rereclaim2(50*sizeof(C_word), 1);
t1=C_restore;}
a=C_alloc(3);
C_initialize_lf(lf,5);
lf[0]=C_h_intern(&lf[0],3,"tak");
lf[1]=C_h_intern(&lf[1],25,"\003sysimplicit-exit-handler");
lf[2]=C_h_intern(&lf[2],4,"exit");
lf[3]=C_h_intern(&lf[3],7,"newline");
lf[4]=C_h_intern(&lf[4],7,"display");
C_register_lf2(lf,5,create_ptable());
t2=(*a=C_CLOSURE_TYPE|2,a[1]=(C_word)f_16,a[2]=t1,tmp=(C_word)a,a+=3,tmp);
C_library_toplevel(2,C_SCHEME_UNDEFINED,t2);}

/* k14 */
static void C_ccall f_16(C_word c,C_word t0,C_word t1){
C_word tmp;
C_word t2;
C_word t3;
C_word ab[3],*a=ab;
C_check_for_interrupt;
if(!C_stack_probe(&a)){
C_save_and_reclaim((void*)tr2,(void*)f_16,2,t0,t1);}
t2=(*a=C_CLOSURE_TYPE|2,a[1]=(C_word)f_19,a[2]=((C_word*)t0)[2],tmp=(C_word)a,a+=3,tmp);
C_eval_toplevel(2,C_SCHEME_UNDEFINED,t2);}

/* k17 in k14 */
static void C_ccall f_19(C_word c,C_word t0,C_word t1){
C_word tmp;
C_word t2;
C_word t3;
C_word t4;
C_word t5;
C_word ab[9],*a=ab;
C_check_for_interrupt;
if(!C_stack_probe(&a)){
C_save_and_reclaim((void*)tr2,(void*)f_19,2,t0,t1);}
t2=C_mutate((C_word*)lf[0]+1 /* (set! tak ...) */,(*a=C_CLOSURE_TYPE|2,a[1]=(C_word)f_21,a[2]=((C_word)li0),tmp=(C_word)a,a+=3,tmp));
t3=(*a=C_CLOSURE_TYPE|2,a[1]=(C_word)f_58,a[2]=((C_word*)t0)[2],tmp=(C_word)a,a+=3,tmp);
t4=(*a=C_CLOSURE_TYPE|2,a[1]=(C_word)f_74,a[2]=t3,tmp=(C_word)a,a+=3,tmp);
C_trace("qtak.ss:9: tak");
((C_proc5)C_fast_retrieve_proc(*((C_word*)lf[0]+1)))(5,*((C_word*)lf[0]+1),t4,C_fix(18),C_fix(12),C_fix(6));}

/* k72 in k17 in k14 */
static void C_ccall f_74(C_word c,C_word t0,C_word t1){
C_word tmp;
C_word t2;
C_word *a;
C_trace("qtak.ss:9: display");
((C_proc3)C_fast_retrieve_proc(*((C_word*)lf[4]+1)))(3,*((C_word*)lf[4]+1),((C_word*)t0)[2],t1);}

/* k56 in k17 in k14 */
static void C_ccall f_58(C_word c,C_word t0,C_word t1){
C_word tmp;
C_word t2;
C_word t3;
C_word ab[3],*a=ab;
C_check_for_interrupt;
if(!C_stack_probe(&a)){
C_save_and_reclaim((void*)tr2,(void*)f_58,2,t0,t1);}
t2=(*a=C_CLOSURE_TYPE|2,a[1]=(C_word)f_61,a[2]=((C_word*)t0)[2],tmp=(C_word)a,a+=3,tmp);
C_trace("qtak.ss:10: newline");
((C_proc2)C_fast_retrieve_proc(*((C_word*)lf[3]+1)))(2,*((C_word*)lf[3]+1),t2);}

/* k59 in k56 in k17 in k14 */
static void C_ccall f_61(C_word c,C_word t0,C_word t1){
C_word tmp;
C_word t2;
C_word t3;
C_word ab[3],*a=ab;
C_check_for_interrupt;
if(!C_stack_probe(&a)){
C_save_and_reclaim((void*)tr2,(void*)f_61,2,t0,t1);}
t2=(*a=C_CLOSURE_TYPE|2,a[1]=(C_word)f_64,a[2]=((C_word*)t0)[2],tmp=(C_word)a,a+=3,tmp);
C_trace("qtak.ss:11: exit");
((C_proc2)C_fast_retrieve_symbol_proc(lf[2]))(2,*((C_word*)lf[2]+1),t2);}

/* k62 in k59 in k56 in k17 in k14 */
static void C_ccall f_64(C_word c,C_word t0,C_word t1){
C_word tmp;
C_word t2;
C_word t3;
C_word t4;
C_word ab[6],*a=ab;
C_check_for_interrupt;
if(!C_stack_probe(&a)){
C_save_and_reclaim((void*)tr2,(void*)f_64,2,t0,t1);}
t2=(*a=C_CLOSURE_TYPE|2,a[1]=(C_word)f_67,a[2]=((C_word*)t0)[2],tmp=(C_word)a,a+=3,tmp);
t3=(*a=C_CLOSURE_TYPE|2,a[1]=(C_word)f_70,a[2]=t2,tmp=(C_word)a,a+=3,tmp);
C_trace("##sys#implicit-exit-handler");
((C_proc2)C_fast_retrieve_symbol_proc(lf[1]))(2,*((C_word*)lf[1]+1),t3);}

/* k68 in k62 in k59 in k56 in k17 in k14 */
static void C_ccall f_70(C_word c,C_word t0,C_word t1){
C_word tmp;
C_word t2;
C_word *a;
t2=t1;
((C_proc2)C_fast_retrieve_proc(t2))(2,t2,((C_word*)t0)[2]);}

/* k65 in k62 in k59 in k56 in k17 in k14 */
static void C_ccall f_67(C_word c,C_word t0,C_word t1){
C_word tmp;
C_word t2;
C_word *a;
t2=((C_word*)t0)[2];
((C_proc2)(void*)(*((C_word*)t2+1)))(2,t2,C_SCHEME_UNDEFINED);}

/* tak in k17 in k14 */
static void C_ccall f_21(C_word c,C_word t0,C_word t1,C_word t2,C_word t3,C_word t4){
C_word tmp;
C_word t5;
C_word t6;
C_word t7;
C_word ab[10],*a=ab;
if(c!=5) C_bad_argc_2(c,5,t0);
C_check_for_interrupt;
if(!C_stack_probe(&a)){
C_save_and_reclaim((void*)tr5,(void*)f_21,5,t0,t1,t2,t3,t4);}
if(C_truep(C_i_greaterp(t2,t3))){
t5=(*a=C_CLOSURE_TYPE|5,a[1]=(C_word)f_35,a[2]=t3,a[3]=t2,a[4]=t4,a[5]=t1,tmp=(C_word)a,a+=6,tmp);
t6=C_a_i_minus(&a,2,t2,C_fix(1));
C_trace("qtak.ss:4: tak");
((C_proc5)C_fast_retrieve_proc(*((C_word*)lf[0]+1)))(5,*((C_word*)lf[0]+1),t5,t6,t3,t4);}
else{
t5=t1;
((C_proc2)(void*)(*((C_word*)t5+1)))(2,t5,t3);}}

/* k33 in tak in k17 in k14 */
static void C_ccall f_35(C_word c,C_word t0,C_word t1){
C_word tmp;
C_word t2;
C_word t3;
C_word t4;
C_word ab[11],*a=ab;
C_check_for_interrupt;
if(!C_stack_probe(&a)){
C_save_and_reclaim((void*)tr2,(void*)f_35,2,t0,t1);}
t2=(*a=C_CLOSURE_TYPE|6,a[1]=(C_word)f_39,a[2]=((C_word*)t0)[2],a[3]=((C_word*)t0)[3],a[4]=((C_word*)t0)[4],a[5]=t1,a[6]=((C_word*)t0)[5],tmp=(C_word)a,a+=7,tmp);
t3=C_a_i_minus(&a,2,((C_word*)t0)[2],C_fix(1));
C_trace("qtak.ss:5: tak");
((C_proc5)C_fast_retrieve_proc(*((C_word*)lf[0]+1)))(5,*((C_word*)lf[0]+1),t2,t3,((C_word*)t0)[4],((C_word*)t0)[3]);}

/* k37 in k33 in tak in k17 in k14 */
static void C_ccall f_39(C_word c,C_word t0,C_word t1){
C_word tmp;
C_word t2;
C_word t3;
C_word t4;
C_word ab[9],*a=ab;
C_check_for_interrupt;
if(!C_stack_probe(&a)){
C_save_and_reclaim((void*)tr2,(void*)f_39,2,t0,t1);}
t2=(*a=C_CLOSURE_TYPE|4,a[1]=(C_word)f_43,a[2]=t1,a[3]=((C_word*)t0)[5],a[4]=((C_word*)t0)[6],tmp=(C_word)a,a+=5,tmp);
t3=C_a_i_minus(&a,2,((C_word*)t0)[4],C_fix(1));
C_trace("qtak.ss:6: tak");
((C_proc5)C_fast_retrieve_proc(*((C_word*)lf[0]+1)))(5,*((C_word*)lf[0]+1),t2,t3,((C_word*)t0)[3],((C_word*)t0)[2]);}

/* k41 in k37 in k33 in tak in k17 in k14 */
static void C_ccall f_43(C_word c,C_word t0,C_word t1){
C_word tmp;
C_word t2;
C_word *a;
C_trace("qtak.ss:3: tak");
((C_proc5)C_fast_retrieve_proc(*((C_word*)lf[0]+1)))(5,*((C_word*)lf[0]+1),((C_word*)t0)[4],((C_word*)t0)[3],((C_word*)t0)[2],t1);}

#ifdef C_ENABLE_PTABLES
static C_PTABLE_ENTRY ptable[14] = {
{"toplevel:qtak_2ess",(void*)C_toplevel},
{"f_16:qtak_2ess",(void*)f_16},
{"f_19:qtak_2ess",(void*)f_19},
{"f_74:qtak_2ess",(void*)f_74},
{"f_58:qtak_2ess",(void*)f_58},
{"f_61:qtak_2ess",(void*)f_61},
{"f_64:qtak_2ess",(void*)f_64},
{"f_70:qtak_2ess",(void*)f_70},
{"f_67:qtak_2ess",(void*)f_67},
{"f_21:qtak_2ess",(void*)f_21},
{"f_35:qtak_2ess",(void*)f_35},
{"f_39:qtak_2ess",(void*)f_39},
{"f_43:qtak_2ess",(void*)f_43},
{NULL,NULL}};
#endif

static C_PTABLE_ENTRY *create_ptable(void){
#ifdef C_ENABLE_PTABLES
return ptable;
#else
return NULL;
#endif
}
/* end of file */

/* ----------------------------------------------
*/
#include "specs.h"
/* -------------------------- */
STATIC void test54()
{
int fork;
char *name  = "t54 dir no access";
char *name1 = "t54 file.txt";
char *name2 = "t54 ro dir";
char *name3  = "t54 --rwx-- dir";
int pdir;
int rdir = 0;
int dir;
int ret;
u_int16_t vol = VolID;
u_int16_t vol2;
DSI *dsi2;
int dt;
		
	enter_test();
    fprintf(stderr,"===================\n");
	fprintf(stderr, "FPRemoveComment:test54: remove comment\n");

	if (!Conn2) {
		test_skipped(T_CONN2);
		goto test_exit;
	}		

	if (!(pdir = no_access_folder(vol, DIRDID_ROOT, name))) {
		goto test_exit;
	}

	dt = FPOpenDT(Conn,vol);
	if (!(rdir = read_only_folder(vol, DIRDID_ROOT, name2) ) ) {
		goto fin;
	}
	dsi2 = &Conn2->dsi;
	vol2  = FPOpenVol(Conn2, Vol);
	if (vol2 == 0xffff) {
		nottested();
		goto fin;
	}
	if (!(dir = FPCreateDir(Conn2,vol2, DIRDID_ROOT , name3))) {
		nottested();
		goto fin;
	}

	FAIL (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name1))
	ret = FPRemoveComment(Conn, vol,  DIRDID_ROOT , name);
	if (not_valid(ret, /* MAC */AFPERR_NOITEM, AFPERR_ACCESS)) {
		failed();
	}
	
	ret = FPRemoveComment(Conn, vol,  DIRDID_ROOT , name2);
	if (not_valid(ret, /* MAC */AFPERR_NOITEM, AFPERR_ACCESS)) {
		failed();
	}

	FAIL (FPAddComment(Conn, vol,  DIRDID_ROOT , name1, "essai")) 
	FAIL (FPRemoveComment(Conn, vol,  DIRDID_ROOT , name1))
	ret = FPRemoveComment(Conn, vol,  DIRDID_ROOT , name1);
	if (not_valid(ret, /* MAC */AFPERR_NOITEM, 0)) {
		failed();
	}

	FAIL (FPAddComment(Conn, vol,  DIRDID_ROOT , name3, "essai")) 
	FAIL (FPRemoveComment(Conn, vol,  DIRDID_ROOT , name3))
	ret = FPRemoveComment(Conn, vol,  DIRDID_ROOT , name3);
	if (not_valid(ret, /* MAC */AFPERR_NOITEM, 0)) {
		failed();
	}

	fork = FPOpenFork(Conn, vol, OPENFORK_DATA  , 0 ,DIRDID_ROOT, name1,OPENACC_RD );
	if (!fork) {
		failed();
		goto fin;
	}

	FAIL (FPAddComment(Conn, vol,  DIRDID_ROOT , name1, "essai")) 
	FAIL (FPRemoveComment(Conn, vol,  DIRDID_ROOT , name1))
	FPCloseFork(Conn,fork);
#if 0
	if (ntohl(AFPERR_NOITEM) != FPRemoveComment(Conn, vol,  DIRDID_ROOT , "bogus folder")) {
		fprintf(stderr,"\tFAILED\n");
	}
#endif
fin:
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name1)) 
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name3)) 
	delete_folder(vol, DIRDID_ROOT, name);
	if (rdir) {
		delete_folder(vol, DIRDID_ROOT, name2);
	}
	FAIL (FPCloseDT(Conn, dt))
test_exit:
	exit_test("test54");
}


/* -------------------------- */
STATIC void test379()
{
int fork;
char *name1 = "t379 file.txt";
u_int16_t vol = VolID;
int  dt;

		
	enter_test();
    fprintf(stderr,"===================\n");
	fprintf(stderr, "FPRemoveComment:test379: remove comment\n");

	FAIL (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name1))

	fork = FPOpenFork(Conn, vol, OPENFORK_DATA  , 0 ,DIRDID_ROOT, name1,OPENACC_RD );
	if (!fork) {
		failed();
		goto fin;
	}
	dt = FPOpenDT(Conn,vol);
	FAIL (FPAddComment(Conn, vol,  DIRDID_ROOT , name1, "essai")) 
	FAIL (FPRemoveComment(Conn, vol,  DIRDID_ROOT , name1))
	FPCloseFork(Conn,fork);
	FAIL (FPCloseDT(Conn, dt))
fin:
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name1)) 
	exit_test("test379");
}


/* ----------- */
void FPRemoveComment_test()
{
    fprintf(stderr,"===================\n");
    fprintf(stderr,"FPRemoveComment page 247\n");
	test54();
	test379();
}


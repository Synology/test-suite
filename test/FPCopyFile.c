/* ----------------------------------------------
*/
#include "specs.h"

/* ------------------------- */
STATIC void test71()
{
int fork;
int dir;
int dir1;
u_int16_t bitmap = 0;
char *name  = "t71 Copy file";
char *name1 = "t71 new file name";
char *name2 = "t71 dir";
char *ndir = "t71 no access";
int pdir = 0;

u_int16_t vol = VolID;
DSI *dsi;

	dsi = &Conn->dsi;

	enter_test();
    fprintf(stderr,"===================\n");
    fprintf(stderr,"FPCopyFile:test71: Copy file\n");
	if (!Conn2) {
		test_skipped(T_CONN2);
		goto test_exit;
	}		

	if (!(pdir = no_access_folder(vol, DIRDID_ROOT, ndir))) {
		goto test_exit;
	}

	if (!(dir = FPCreateDir(Conn,vol, DIRDID_ROOT , name2))) {
		nottested();
		goto test_exit;
	}

	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name2)) 

	/* sdid bad */
	FAIL (ntohl(AFPERR_NOOBJ) != FPCopyFile(Conn, vol, dir, vol, DIRDID_ROOT, name, name1))

	FPCloseVol(Conn,vol);
	vol  = FPOpenVol(Conn, Vol);
	/* cname unchdirable */
	FAIL (ntohl(AFPERR_BADTYPE) != FPCopyFile(Conn, vol, DIRDID_ROOT, vol, DIRDID_ROOT, ndir, name1)) 
	/* second time once bar is in the cache */
	FAIL (ntohl(AFPERR_BADTYPE) != FPCopyFile(Conn, vol, DIRDID_ROOT, vol, DIRDID_ROOT, ndir, name1)) 

	if (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name)){
		failed();
		goto fin;
	}

	FAIL (!(dir1 = FPCreateDir(Conn,vol, DIRDID_ROOT , name2))) 

	/* source is a dir */
	FAIL (ntohl(AFPERR_BADTYPE) != FPCopyFile(Conn, vol, DIRDID_ROOT, vol, DIRDID_ROOT, name2, name1)) 

	fork = FPOpenFork(Conn, vol, OPENFORK_DATA , bitmap ,DIRDID_ROOT, name,OPENACC_WR | OPENACC_RD);
	if (fork) {
	
		FAIL (FPCopyFile(Conn, vol, DIRDID_ROOT, vol, DIRDID_ROOT, name, name1))
		FAIL (FPCloseFork(Conn,fork))
		FAIL (FPDelete(Conn, vol,  DIRDID_ROOT, name1))
	}	
	else {
		failed();
	}
	/* dvol bad */
	FAIL (ntohl(AFPERR_PARAM) != FPCopyFile(Conn, vol, DIRDID_ROOT, vol +1, dir, name, name1)) 

	/* ddid bad */
	FAIL (ntohl(AFPERR_NOOBJ) != FPCopyFile(Conn, vol, DIRDID_ROOT , vol, dir,  name, name1)) 

	/* ok */
	FAIL (FPCopyFile(Conn, vol, DIRDID_ROOT, vol, DIRDID_ROOT, name, name1)) 

	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT, name))
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT, name1))
fin:	
	delete_folder(vol, DIRDID_ROOT, ndir);
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT, name2))
test_exit:
	exit_test("test71");
}

/* ------------------------- */
STATIC void test158()
{
char *name  = "t158 old file name";
char *name1 = "t158 new file name";
u_int16_t vol = VolID;

	enter_test();
    fprintf(stderr,"===================\n");
    fprintf(stderr,"FPCopyFile:test158: copyFile dest exist\n");

	if (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name)) {
		nottested();
	}
	if (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name1)) {
		nottested();
	}

	/* sdid bad */
	FAIL (ntohl(AFPERR_EXIST) != FPCopyFile(Conn, vol, DIRDID_ROOT, vol, DIRDID_ROOT, name, name1))
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name)) 
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name1))

	exit_test("test158");
}

/* ------------------------- */
STATIC void test315()
{
u_int16_t bitmap = 0;
char *name  = "t315 old file name";
char *name1 = "t315 new file name";
u_int16_t vol = VolID;
int fork;

	enter_test();
    fprintf(stderr,"===================\n");
    fprintf(stderr,"FPCopyFile:test315: copyFile\n");

	if (get_vol_free(vol) < 130*1024*1024) {
	    /* assume sparse file for setforkparam, not for copyfile */
		test_skipped(T_VOL_SMALL);
		goto test_exit;
	}		

	if (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name)){
		nottested();
		goto test_exit;
	}

	fork = FPOpenFork(Conn, vol, OPENFORK_DATA , bitmap ,DIRDID_ROOT, name, OPENACC_WR | OPENACC_RD);
	if (!fork) {
		nottested();
		goto fin;
	}		

	if (FPSetForkParam(Conn, fork, (1<<FILPBIT_DFLEN), 64*1024*1024)) {
		nottested();
		FPCloseFork(Conn,fork);
		goto fin;
	}

	FAIL (FPCloseFork(Conn,fork))

	FAIL (FPCopyFile(Conn, vol, DIRDID_ROOT, vol, DIRDID_ROOT, name, name1))
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT, name1))

	fork = FPOpenFork(Conn, vol, OPENFORK_DATA , bitmap ,DIRDID_ROOT, name, OPENACC_WR | OPENACC_RD);
	if (!fork) {
		nottested();
		goto fin;
	}		

	if (FPSetForkParam(Conn, fork, (1<<FILPBIT_DFLEN), 129*1024*1024)) {
		nottested();
		FPCloseFork(Conn,fork);
		goto fin;
	}

	FAIL (FPCloseFork(Conn,fork))

	FAIL (FPCopyFile(Conn, vol, DIRDID_ROOT, vol, DIRDID_ROOT, name, name1))
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT, name1))

fin:	
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT, name))
test_exit:
	exit_test("test315");
}

/* ------------------------- */
STATIC void test317()
{
char *name  = "t317 old file name";
char *name1 = "t317 new file name";
u_int16_t vol = VolID;
int tp,tp1;
int  ofs =  3 * sizeof( u_int16_t );
struct afp_filedir_parms filedir;
DSI *dsi = &Conn->dsi; 
u_int16_t bitmap;
char finder_info[32];

	enter_test();
    fprintf(stderr,"===================\n");
    fprintf(stderr,"FPCopyFile:test317: copyFile check meta data\n");

	if (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name)) {
		nottested();
		goto fin;
	}
	tp = get_fid(Conn, vol, DIRDID_ROOT, name);
	if (!tp) {
		nottested();
		goto fin;
	}
	bitmap = (1 << FILPBIT_FINFO);
	if (FPGetFileDirParams(Conn, vol,  DIRDID_ROOT , name, bitmap,0)) {
		failed();
	}
	else {
		filedir.isdir = 0;
		afp_filedir_unpack(&filedir, dsi->data +ofs, bitmap, 0);
		memcpy(filedir.finder_info, "PDF CARO", 8);
		
		memcpy(finder_info, filedir.finder_info, 32);
 		FAIL (FPSetFileParams(Conn, vol, DIRDID_ROOT , name, bitmap, &filedir)) 
	    FAIL (FPGetFileDirParams(Conn, vol,  DIRDID_ROOT , name, bitmap,0))
	}
	
	FAIL (FPCopyFile(Conn, vol, DIRDID_ROOT, vol, DIRDID_ROOT, name, name1))

	if (FPGetFileDirParams(Conn, vol,  DIRDID_ROOT , name1, bitmap,0)) {
		failed();
	}
	else {
		filedir.isdir = 0;
		afp_filedir_unpack(&filedir, dsi->data +ofs, bitmap, 0);
		if (memcmp(finder_info, filedir.finder_info, 32)) {
	        fprintf(stderr,"\tFAILED finder info differ\n");  
	        failed_nomsg();
	        goto fin;
		}
	}

	tp1 = get_fid(Conn, vol, DIRDID_ROOT, name1);
	if (!tp1) {
		nottested();
		goto fin;
	}
	if (tp == tp1) {
	    fprintf(stderr,"\tFAILED both files have same ID\n");  
	    failed_nomsg();
	}

fin:
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name)) 
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name1))

	exit_test("test317");
}

/* ------------------------- */
STATIC void test332()
{
char *name  = "t332 old file name";
char *name1 = "t332 new file name";
u_int16_t vol = VolID;
int tp,tp1;
int  ofs =  3 * sizeof( u_int16_t );
struct afp_filedir_parms filedir;
DSI *dsi = &Conn->dsi; 
u_int16_t bitmap;
u_int32_t mdate = 0;

	enter_test();
    fprintf(stderr,"===================\n");
    fprintf(stderr,"FPCopyFile:test332: copyFile check meta data\n");

	if (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name)) {
		nottested();
		goto fin;
	}
	fprintf(stderr,"sleep(2)\n");  
	sleep(2);
	tp = get_fid(Conn, vol, DIRDID_ROOT, name);
	if (!tp) {
		nottested();
		goto fin;
	}
	bitmap = (1<<DIRPBIT_MDATE);
	if (FPGetFileDirParams(Conn, vol,  DIRDID_ROOT , name, bitmap,0)) {
		failed();
	}
	else {
		filedir.isdir = 0;
		afp_filedir_unpack(&filedir, dsi->data +ofs, bitmap, 0);
		mdate = filedir.mdate;
	}
	
	FAIL (FPCopyFile(Conn, vol, DIRDID_ROOT, vol, DIRDID_ROOT, name, name1))

	if (FPGetFileDirParams(Conn, vol,  DIRDID_ROOT , name1, bitmap,0)) {
		failed();
	}
	else {
		filedir.isdir = 0;
		afp_filedir_unpack(&filedir, dsi->data +ofs, bitmap, 0);
		if (mdate != filedir.mdate)  {
	        fprintf(stderr,"\tFAILED modification date differ\n");  
	        failed_nomsg();
	        goto fin;
		}
	}

	tp1 = get_fid(Conn, vol, DIRDID_ROOT, name1);
	if (!tp1) {
		nottested();
		goto fin;
	}
	if (tp == tp1) {
	    fprintf(stderr,"\tFAILED both files have same ID\n");  
	    failed_nomsg();
	}

fin:
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name)) 
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name1))

	exit_test("test332");
}

/* ----------- */
STATIC void test374()
{
int fork;
u_int16_t vol2;
u_int16_t bitmap = 0;
char *name  = "t374 Copy file";
char *name1 = "t374 new file name";
u_int16_t vol = VolID;
DSI *dsi;

	dsi = &Conn->dsi;

	enter_test();
    fprintf(stderr,"===================\n");
    fprintf(stderr,"FPCopyFile:test374: Copy open file, two clients\n");
	if (!Conn2) {
		test_skipped(T_CONN2);
		goto test_exit;
	}		

	if (Locking) {
		test_skipped(T_LOCKING);
		goto test_exit;
	}

	vol2  = FPOpenVol(Conn2, Vol);
	if (vol2 == 0xffff) {
		nottested();
		goto test_exit;
	}

	if (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name)){
		failed();
		goto fin;
	}

	fork = FPOpenFork(Conn, vol, OPENFORK_DATA , bitmap ,DIRDID_ROOT, name,OPENACC_WR | OPENACC_DRD);
	if (fork) {
		FAIL (ntohl(AFPERR_DENYCONF) != FPCopyFile(Conn2, vol2, DIRDID_ROOT, vol2, DIRDID_ROOT, name, name1))
		FAIL (FPCloseFork(Conn,fork))
	}	
	else {
		failed();
	}

	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT, name))
	FPDelete(Conn, vol,  DIRDID_ROOT, name1);
fin:
	FPCloseVol(Conn2,vol2);

test_exit:
	exit_test("test374");
}

/* ------------------------- */
STATIC void test375()
{
int fork;
int fork1;
u_int16_t bitmap = 0;
char *name  = "t375 old file name";
char *name1 = "t375 new file name";
u_int16_t vol = VolID;

	enter_test();
    fprintf(stderr,"===================\n");
    fprintf(stderr,"FPCopyFile:test158: copyFile dest exist and is open\n");

	if (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name)) {
		nottested();
	}
	if (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name1)) {
		nottested();
	}

	fork = FPOpenFork(Conn, vol, OPENFORK_DATA , bitmap ,DIRDID_ROOT, name1,OPENACC_WR | OPENACC_DWR);
	if (!fork) {
		failed();
		goto fin;
	}

	fork1 = FPOpenFork(Conn, vol, OPENFORK_DATA , bitmap ,DIRDID_ROOT, name1,OPENACC_WR);
	if (fork1) {
		failed();
		FAIL (FPCloseFork(Conn,fork1))
	}

	FAIL (ntohl(AFPERR_EXIST) != FPCopyFile(Conn, vol, DIRDID_ROOT, vol, DIRDID_ROOT, name, name1))
	fork1 = FPOpenFork(Conn, vol, OPENFORK_DATA , bitmap ,DIRDID_ROOT, name1,OPENACC_WR);
	if (fork1) {
		failed();
		FAIL (FPCloseFork(Conn,fork1))
	}

	FAIL (FPCloseFork(Conn,fork))

fin:
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name)) 
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name1))

	exit_test("test375");
}

/* ------------------------- */
STATIC void test401()
{
int  dir = 0;
char *name = "t401 file.pdf";
char *name1= "new t401 file.pdf";

char *ndir = "t401 dir";
u_int16_t vol = VolID;
int  ofs =  3 * sizeof( u_int16_t );
struct afp_filedir_parms filedir;
u_int16_t bitmap = 0;
int fork;
DSI *dsi;

	dsi = &Conn->dsi;

	enter_test();
    fprintf(stderr,"===================\n");
    fprintf(stderr,"FPCopyFile:t401: unix access privilege, read only file\n");

	if ( !(get_vol_attrib(vol) & VOLPBIT_ATTR_UNIXPRIV)) {
		test_skipped(T_UNIX_PREV);
		goto test_exit;
	}

	if (!(dir = FPCreateDir(Conn,vol, DIRDID_ROOT , ndir))) {
		nottested();
		goto test_exit;
	}
	filedir.isdir = 1;
	afp_filedir_unpack(&filedir, dsi->data +ofs, bitmap, 0);

	bitmap = (1<< DIRPBIT_UNIXPR);
	filedir.unix_priv = 0770;
    filedir.access[0] = 0;
    filedir.access[1] = filedir.access[2] = filedir.access[3] = 0;
 	FAIL (FPSetFilDirParam(Conn, vol, dir , "", bitmap, &filedir)) 

	if (FPCreateFile(Conn, vol,  0, dir , name)) {
		nottested();
		goto fin;
	}

	fork = FPOpenFork(Conn, vol, OPENFORK_DATA , 0, dir, name, OPENACC_WR |OPENACC_RD);
	if (!fork) {
		nottested();
		goto fin;
	}
	if (FPSetForkParam(Conn, fork, (1<<FILPBIT_DFLEN), 400)) {
		FPCloseFork(Conn,fork);
		nottested();
		goto fin;
	}
	FPCloseFork(Conn,fork);
	
	fork = FPOpenFork(Conn, vol, OPENFORK_RSCS , 0, dir, name, OPENACC_WR |OPENACC_RD);
	if (!fork) {
		nottested();
		goto fin;
	}
	if (FPSetForkParam(Conn, fork, (1<<FILPBIT_RFLEN), 300)) {
		FPCloseFork(Conn,fork);
		nottested();
		goto fin;
	}
	FPCloseFork(Conn,fork);
	
	bitmap = (1 <<  FILPBIT_PDINFO) | (1<< FILPBIT_PDID) | (1<< FILPBIT_FNUM) |
		(1 << DIRPBIT_UNIXPR) | (1<<FILPBIT_ATTR);

	if (FPGetFileDirParams(Conn, vol, dir, name, bitmap, 0)) {
	    failed();
	    goto fin1;
	}
	filedir.isdir = 0;
	afp_filedir_unpack(&filedir, dsi->data +ofs, bitmap, 0);

	bitmap = (1<< DIRPBIT_UNIXPR);
	filedir.unix_priv = 0444;
    filedir.access[0] = 0;
    filedir.access[1] = filedir.access[2] = filedir.access[3] = 0;
 	FAIL (FPSetFilDirParam(Conn, vol, dir , name, bitmap, &filedir)) 

	bitmap = (1 <<  FILPBIT_PDINFO) | (1<< FILPBIT_PDID) | (1<< FILPBIT_FNUM) |
		(1 << DIRPBIT_UNIXPR) | (1<<FILPBIT_ATTR);
	if (FPGetFileDirParams(Conn, vol, dir, name, bitmap, 0)) {
	    failed();
	    goto fin1;
	}
	FAIL (FPCopyFile(Conn, vol, dir, vol, dir, name, name1))

	FAIL (FPDelete(Conn, vol,  dir , name1))

fin1:
	FAIL (FPDelete(Conn, vol,  dir , name))
fin:	
	FAIL (FPDelete(Conn, vol,  dir , ""))
test_exit:
	exit_test("test401");
}

/* ------------------------- */
STATIC void test402()
{
int  dir = 0;
char *name = "t402 file.pdf";
char *name1= "new t402 file.pdf";

char *ndir = "t402 dir";
u_int16_t vol = VolID;
int  ofs =  3 * sizeof( u_int16_t );
struct afp_filedir_parms filedir;
u_int16_t bitmap = 0;
int fork;
DSI *dsi;

	dsi = &Conn->dsi;

	enter_test();
    fprintf(stderr,"===================\n");
    fprintf(stderr,"FPCopyFile:t401: unix access privilege, read only file\n");

	if ( !(get_vol_attrib(vol) & VOLPBIT_ATTR_UNIXPRIV)) {
		test_skipped(T_UNIX_PREV);
		goto test_exit;
	}

	if (!(dir = FPCreateDir(Conn,vol, DIRDID_ROOT , ndir))) {
		nottested();
		goto test_exit;
	}

	if (FPCreateFile(Conn, vol,  0, dir , name)) {
		nottested();
		goto fin;
	}

	fork = FPOpenFork(Conn, vol, OPENFORK_DATA , 0, dir, name, OPENACC_WR |OPENACC_RD);
	if (!fork) {
		nottested();
		goto fin;
	}
	if (FPSetForkParam(Conn, fork, (1<<FILPBIT_DFLEN), 400)) {
		FPCloseFork(Conn,fork);
		nottested();
		goto fin;
	}
	FPCloseFork(Conn,fork);
	
	fork = FPOpenFork(Conn, vol, OPENFORK_RSCS , 0, dir, name, OPENACC_WR |OPENACC_RD);
	if (!fork) {
		nottested();
		goto fin;
	}
	if (FPSetForkParam(Conn, fork, (1<<FILPBIT_RFLEN), 300)) {
		FPCloseFork(Conn,fork);
		nottested();
		goto fin;
	}
	FPCloseFork(Conn,fork);
	
	bitmap = (1 <<  FILPBIT_PDINFO) | (1<< FILPBIT_PDID) | (1<< FILPBIT_FNUM) |
		(1 << DIRPBIT_UNIXPR) | (1<<FILPBIT_ATTR);

	if (FPGetFileDirParams(Conn, vol, dir, name, bitmap, 0)) {
	    failed();
	    goto fin1;
	}
	filedir.isdir = 0;
	afp_filedir_unpack(&filedir, dsi->data +ofs, bitmap, 0);

	bitmap = (1<< DIRPBIT_UNIXPR);
	filedir.unix_priv = 0222;
    filedir.access[0] = 0;
    filedir.access[1] = filedir.access[2] = filedir.access[3] = 0;
 	FAIL (FPSetFilDirParam(Conn, vol, dir , name, bitmap, &filedir)) 

	bitmap = (1 <<  FILPBIT_PDINFO) | (1<< FILPBIT_PDID) | (1<< FILPBIT_FNUM) |
		(1 << DIRPBIT_UNIXPR) | (1<<FILPBIT_ATTR);
	if (FPGetFileDirParams(Conn, vol, dir, name, bitmap, 0)) {
	    failed();
	    goto fin1;
	}
	FAIL (ntohl(AFPERR_DENYCONF) != FPCopyFile(Conn, vol, dir, vol, dir, name, name1))

fin1:
	FAIL (FPDelete(Conn, vol,  dir , name))
fin:	
	FAIL (FPDelete(Conn, vol,  dir , ""))
test_exit:
	exit_test("test402");
}

/* ------------------------- */
STATIC void test403()
{
int  dir = 0;
char *name = "t403 file.pdf";
char *name1= "new t403 file.pdf";

char *ndir = "t403 dir";
u_int16_t vol = VolID;
int  ofs =  3 * sizeof( u_int16_t );
struct afp_filedir_parms filedir;
u_int16_t bitmap = 0;
DSI *dsi;

	dsi = &Conn->dsi;

	enter_test();
    fprintf(stderr,"===================\n");
    fprintf(stderr,"FPCopyFile:t403: unix access privilege, same priv\n");

	if ( !(get_vol_attrib(vol) & VOLPBIT_ATTR_UNIXPRIV)) {
		test_skipped(T_UNIX_PREV);
		goto test_exit;
	}

	if (!(dir = FPCreateDir(Conn,vol, DIRDID_ROOT , ndir))) {
		nottested();
		goto test_exit;
	}
	filedir.isdir = 1;
	afp_filedir_unpack(&filedir, dsi->data +ofs, bitmap, 0);

	bitmap = (1<< DIRPBIT_UNIXPR);
	filedir.unix_priv = 0770;
    filedir.access[0] = 0;
    filedir.access[1] = filedir.access[2] = filedir.access[3] = 0;
 	FAIL (FPSetFilDirParam(Conn, vol, dir , "", bitmap, &filedir)) 

	if (FPCreateFile(Conn, vol,  0, dir , name)) {
		nottested();
		goto fin;
	}

	bitmap = (1 <<  FILPBIT_PDINFO) | (1<< FILPBIT_PDID) | (1<< FILPBIT_FNUM) |
		(1 << DIRPBIT_UNIXPR) | (1<<FILPBIT_ATTR);

	if (FPGetFileDirParams(Conn, vol, dir, name, bitmap, 0)) {
	    failed();
	    goto fin1;
	}
	filedir.isdir = 0;
	afp_filedir_unpack(&filedir, dsi->data +ofs, bitmap, 0);

	bitmap = (1<< DIRPBIT_UNIXPR);
	filedir.unix_priv = 0444;
    filedir.access[0] = 0;
    filedir.access[1] = filedir.access[2] = filedir.access[3] = 0;
 	FAIL (FPSetFilDirParam(Conn, vol, dir , name, bitmap, &filedir)) 

	bitmap = (1 <<  FILPBIT_PDINFO) | (1<< FILPBIT_PDID) | (1<< FILPBIT_FNUM) |
		(1 << DIRPBIT_UNIXPR) | (1<<FILPBIT_ATTR);
	if (FPGetFileDirParams(Conn, vol, dir, name, bitmap, 0)) {
	    failed();
	    goto fin1;
	}

	FAIL (FPCopyFile(Conn, vol, dir, vol, dir, name, name1))

	bitmap = (1 <<  FILPBIT_PDINFO) | (1<< FILPBIT_PDID) | (1<< FILPBIT_FNUM) |
		(1 << DIRPBIT_UNIXPR) | (1<<FILPBIT_ATTR);

	if (FPGetFileDirParams(Conn, vol, dir, name1, bitmap, 0)) {
	    failed();
	    goto fin1;
	}
	filedir.isdir = 0;
	afp_filedir_unpack(&filedir, dsi->data +ofs, bitmap, 0);
	if ((filedir.unix_priv & 0444) != 0444) {
		fprintf(stderr,"\tFAILED unix priv differ\n");
	    failed_nomsg();
	}

	FAIL (FPDelete(Conn, vol,  dir , name1))

fin1:
	FAIL (FPDelete(Conn, vol,  dir , name))
fin:	
	FAIL (FPDelete(Conn, vol,  dir , ""))
test_exit:
	exit_test("test403");
}


/* ----------- */
void FPCopyFile_test()
{
    fprintf(stderr,"===================\n");
    fprintf(stderr,"FPCopyFile page 131\n");
    test71();
	test158();
	test315();
	test317();
	test332();
	test374();
	test375();
	test401();
	test402();
	test403();
}


/* ----------------------------------------------
*/
#include "specs.h"

/* ----------------------- */
STATIC void test38()
{
u_int16_t bitmap = (1<< DIRPBIT_DID);
int  rdir;
int  did;
u_int16_t vol = VolID;
char *name = "t38 read only access dir";
char *nfile = "t38 read write file";
DSI *dsi;

	if (!Conn2) {
		test_skipped(T_CONN2);
		return;
	}		

	dsi = &Conn->dsi;
    fprintf(stderr,"===================\n");
    fprintf(stderr,"FPEnumerate:test38: enumerate folder with no write access\n");
	if (!Conn2) {
		test_skipped(T_CONN2);
		return;
	}		

	if (!(rdir = read_only_folder_with_file(vol, DIRDID_ROOT, name, nfile) ) ) {
		return;
	}
	
	if (FPGetFileDirParams(Conn, vol,  DIRDID_ROOT, name, 0, bitmap)) {
		failed();
		goto fin;
	}
	memcpy(&did, dsi->data +3 * sizeof( u_int16_t ), sizeof(did));

	if (FPEnumerate(Conn, vol,  DIRDID_ROOT , name, 
	         (1<<FILPBIT_LNAME) | (1<<FILPBIT_FNUM ) | (1<<FILPBIT_ATTR) | (1<<FILPBIT_FINFO)|
	         (1<<FILPBIT_CDATE) | (1<<FILPBIT_BDATE) | (1<<FILPBIT_MDATE) |
	         (1<<FILPBIT_DFLEN) | (1<<FILPBIT_RFLEN)
	         ,
		     (1<< DIRPBIT_ATTR) |  (1<<DIRPBIT_ATTR) | (1<<DIRPBIT_FINFO) |
	         (1<<DIRPBIT_CDATE) | (1<<DIRPBIT_BDATE) | (1<<DIRPBIT_MDATE) |
		    (1<< DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID)|(1<< DIRPBIT_ACCESS)
		)) {
		failed();
		goto fin;
	}

	if (FPEnumerate(Conn, vol,  did , "", 
	         (1<<FILPBIT_LNAME) | (1<<FILPBIT_FNUM ) | (1<<FILPBIT_ATTR) | (1<<FILPBIT_FINFO)|
	         (1<<FILPBIT_CDATE) | (1<<FILPBIT_BDATE) | (1<<FILPBIT_MDATE)
	         ,
		     (1<< DIRPBIT_ATTR) |  (1<<DIRPBIT_ATTR) | (1<<DIRPBIT_FINFO) |
	         (1<<DIRPBIT_CDATE) | (1<<DIRPBIT_BDATE) | (1<<DIRPBIT_MDATE) |
		    (1<< DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID)|(1<< DIRPBIT_ACCESS)
		)) {
		failed();
		goto fin;
	}
#if 0
	bitmap = 0;
	fork = FPOpenFork(Conn, vol, OPENFORK_DATA , bitmap , did, "toto.txt",OPENACC_WR | OPENACC_RD);

	if (fork) {
		fprintf(stderr,"\tFAILED\n");
		return;
	}		

	fork = FPOpenFork(Conn, vol, OPENFORK_DATA , bitmap, did, "toto.txt", OPENACC_RD);

	if (!fork) {
		fprintf(stderr,"\tFAILED\n");
		return;
	}		
	FPCloseFork(Conn,fork);
#endif
fin:
	delete_folder_with_file(vol, DIRDID_ROOT, name, nfile);
}

/* ------------------------- */
#if 0
STATIC void test34()
{
char *name = "essai permission";

    fprintf(stderr,"===================\n");
    fprintf(stderr,"FPEnumerate:test34: folder with --rwx-- perm\n");

	if (ntohl(AFPERR_ACCESS) != FPGetFileDirParams(Conn, vol, DIRDID_ROOT, name, 0, 
	    (1 <<  DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID) |
	    (1 << DIRPBIT_ACCESS))) 
	{
		fprintf(stderr,"\tFAILED\n");
		return;
	}

	FPEnumerate(Conn, vol,  DIRDID_ROOT , "", 
	     (1<< FILPBIT_LNAME) | (1<< FILPBIT_FNUM ),
		 (1<< DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID)|(1<< DIRPBIT_ACCESS)
		);

	if (ntohl(AFPERR_ACCESS) != FPGetFileDirParams(Conn, vol, DIRDID_ROOT, name, 0, 
	     (1 <<  DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID) |
	     (1 << DIRPBIT_ACCESS))
	   ) 
	{
		fprintf(stderr,"\tFAILED\n");
		return;
	}
fin:
}
#endif
/* ------------------------- */
STATIC void test40()
{
char *name  = "t40 dir";
char *name1 = "t40 file";
u_int16_t vol = VolID;
unsigned int ret;
int  dir;

    fprintf(stderr,"===================\n");
    fprintf(stderr,"FPEnumerate:test40: enumerate deleted folder\n");

	dir   = FPCreateDir(Conn,vol, DIRDID_ROOT , name);

	if (!dir) {
		nottested();
		return;
	}
	
	FAIL (FPCreateFile(Conn, vol,  0, dir , name1)) 

	if (FPEnumerate(Conn, vol,  dir , "", 
	         (1<<FILPBIT_LNAME) | (1<<FILPBIT_FNUM ) | (1<<FILPBIT_ATTR) | (1<<FILPBIT_FINFO)|
	         (1<<FILPBIT_CDATE) | (1<<FILPBIT_BDATE) | (1<<FILPBIT_MDATE)
	         ,
		     (1<< DIRPBIT_ATTR) | (1<<DIRPBIT_FINFO) |
	         (1<<DIRPBIT_CDATE) | (1<<DIRPBIT_BDATE) | (1<<DIRPBIT_MDATE) |
		    (1<< DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID)|(1<< DIRPBIT_ACCESS)
		)
	) {
		failed();
	}

	FAIL (FPDelete(Conn, vol,  dir, name1))

	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT, name))

	if (ntohl(AFPERR_NOOBJ) != FPGetFileDirParams(Conn, vol,  dir, "", 0, 
			(1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID)|(1<< DIRPBIT_ACCESS))) 
	{
		failed();
	}
	ret = FPEnumerate(Conn, vol,  dir , "", 
	         (1<<FILPBIT_LNAME) | (1<<FILPBIT_FNUM ) | (1<<FILPBIT_ATTR) | (1<<FILPBIT_FINFO)|
	         (1<<FILPBIT_CDATE) | (1<<FILPBIT_BDATE) | (1<<FILPBIT_MDATE)
	         ,
		     (1<< DIRPBIT_ATTR) | (1<<DIRPBIT_FINFO) |
	         (1<<DIRPBIT_CDATE) | (1<<DIRPBIT_BDATE) | (1<<DIRPBIT_MDATE) |
		    (1<< DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID)|(1<< DIRPBIT_ACCESS)
		);
	if (not_valid_bitmap(ret, BITERR_NOOBJ | BITERR_NODIR, AFPERR_NODIR)) {
		failed();
	}
	
}

/* ------------------------- */
STATIC void test41()
{
int dir = 0;
u_int16_t vol = VolID;
char *name  = "t41 dir";
char *name1 = "t41 dir/file";
char *name2 = "t41 dir/sub dir/foo";
char *name3 = "t41 dir/sub dir";
unsigned int ret;

    fprintf(stderr,"===================\n");
    fprintf(stderr,"FPEnumerate:test41: enumerate folder not there\n");

	dir   = FPCreateDir(Conn,vol, DIRDID_ROOT , name);
	if (!dir) {
		nottested();
		return;
	}
	if (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name1)) {
		nottested();
		goto fin;
	}

	if (FPEnumerate(Conn, vol,  dir , "", 
	         (1<<FILPBIT_LNAME) | (1<<FILPBIT_FNUM ) | (1<<FILPBIT_ATTR) | (1<<FILPBIT_FINFO)|
	         (1<<FILPBIT_CDATE) | (1<<FILPBIT_BDATE) | (1<<FILPBIT_MDATE)
	         ,
		     (1<< DIRPBIT_ATTR) | (1<<DIRPBIT_FINFO) |
	         (1<<DIRPBIT_CDATE) | (1<<DIRPBIT_BDATE) | (1<<DIRPBIT_MDATE) |
		    (1<< DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID)|(1<< DIRPBIT_ACCESS)
		)
	) {
		nottested();
		goto fin;
	}
	
	ret = FPEnumerate(Conn, vol,  DIRDID_ROOT , name2, 
	         (1<<FILPBIT_LNAME) | (1<<FILPBIT_FNUM ) | (1<<FILPBIT_ATTR) | (1<<FILPBIT_FINFO)|
	         (1<<FILPBIT_CDATE) | (1<<FILPBIT_BDATE) | (1<<FILPBIT_MDATE)
	         ,
		     (1<< DIRPBIT_ATTR) | (1<<DIRPBIT_FINFO) |
	         (1<<DIRPBIT_CDATE) | (1<<DIRPBIT_BDATE) | (1<<DIRPBIT_MDATE) |
		    (1<< DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID)|(1<< DIRPBIT_ACCESS)
		);

	if (not_valid_bitmap(ret, BITERR_NOOBJ | BITERR_NODIR, AFPERR_NODIR)) {
		failed();
	}
	
	ret = FPEnumerate(Conn, vol,  DIRDID_ROOT , name3, 
	         (1<<FILPBIT_LNAME) | (1<<FILPBIT_FNUM ) | (1<<FILPBIT_ATTR) | (1<<FILPBIT_FINFO)|
	         (1<<FILPBIT_CDATE) | (1<<FILPBIT_BDATE) | (1<<FILPBIT_MDATE)
	         ,
		     (1<< DIRPBIT_ATTR) | (1<<DIRPBIT_FINFO) |
	         (1<<DIRPBIT_CDATE) | (1<<DIRPBIT_BDATE) | (1<<DIRPBIT_MDATE) |
		    (1<< DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID)|(1<< DIRPBIT_ACCESS)
		);

	if (not_valid_bitmap(ret, BITERR_NOOBJ | BITERR_NODIR, AFPERR_NODIR)) {
		failed();
	}

	if (ntohl(AFPERR_PARAM) != FPEnumerate(Conn, vol,  0 , "", 
	         (1<<FILPBIT_LNAME) | (1<<FILPBIT_FNUM ) | (1<<FILPBIT_ATTR) | (1<<FILPBIT_FINFO)|
	         (1<<FILPBIT_CDATE) | (1<<FILPBIT_BDATE) | (1<<FILPBIT_MDATE)
	         ,
		     (1<< DIRPBIT_ATTR) | (1<<DIRPBIT_FINFO) |
	         (1<<DIRPBIT_CDATE) | (1<<DIRPBIT_BDATE) | (1<<DIRPBIT_MDATE) |
		    (1<< DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID)|(1<< DIRPBIT_ACCESS)
		)
	) {
		failed();
	}
fin:
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT, name1))
	FAIL (FPDelete(Conn, vol,  dir, ""))

}

/* -------------------------------------- */
STATIC void test93()
{
char *name = "t93 bad enumerate file";
char *name1 = "t93 bad enumerate dir";
int dir = 0;
int dir1 = 0;
int ret;
u_int16_t vol = VolID;

    fprintf(stderr,"===================\n");
    fprintf(stderr,"FPEnumerate:test93: enumerate error\n");

	if (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name)) {
		nottested();
		return;
	}
	if (!(dir = FPCreateDir(Conn,vol, DIRDID_ROOT , name1))) {
		nottested();
		goto fin;
	}
	if (!(dir1 = FPCreateDir(Conn,vol, dir , name1))) {
		nottested();
		goto fin;
	}

	ret = FPDelete(Conn, vol,  DIRDID_ROOT , "");
	if (not_valid_bitmap(ret, BITERR_PARAM | BITERR_BUSY, AFPERR_ACCESS)) {
		failed();
		goto fin;
	}

	ret = FPEnumerate(Conn, vol,  DIRDID_ROOT , name, 
	     (1<<FILPBIT_LNAME) | (1<<FILPBIT_FNUM ) | (1<<FILPBIT_ATTR) | (1<<FILPBIT_FINFO)|
	     (1<<FILPBIT_CDATE) | (1<< FILPBIT_PDID)
	      ,
		 (1<< DIRPBIT_ATTR) |
		 (1<< DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID)|(1<< DIRPBIT_ACCESS)
		);

	if (not_valid(ret, /* MAC */AFPERR_NODIR, AFPERR_BADTYPE)) {
		failed();
		goto fin;
	}
	FAIL (htonl( AFPERR_DIRNEMPT) != FPDelete(Conn, vol,  DIRDID_ROOT , name1))

	FAIL (htonl(AFPERR_BITMAP) != FPEnumerate(Conn, vol,  DIRDID_ROOT , name1, 0,0)) 

fin:
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name))

	FAIL (dir1 && FPDelete(Conn, vol,  dir , name1))
	FAIL (dir && FPDelete(Conn, vol,  DIRDID_ROOT , name1))
}

/* ------------------------- */
STATIC void test218()
{
u_int16_t bitmap = 0;
char *name  = "t218 enumerate file";
char *ndir1  = "t218 enumerate dir1";
char *ndir  = "t218 enumerate dir";
u_int16_t vol = VolID;
int  ofs =  4 * sizeof( u_int16_t );
struct afp_filedir_parms filedir;
DSI *dsi;
unsigned int ret;
int dir;
int dir1;
int isdir;

	dsi = &Conn->dsi;

    fprintf(stderr,"===================\n");
    fprintf(stderr,"FPEnumerate:test218: enumerate arguments\n");
	if (Conn->afp_version < 30) {
		test_skipped(T_AFP3);
		return;
	}

	bitmap = (1 << FILPBIT_LNAME);
	if (htonl(AFPERR_NOOBJ) != FPEnumerateFull(Conn, vol, 1, 1, 800,  DIRDID_ROOT, "", bitmap, bitmap)) {
		nottested();
		return;
	}
	if (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name)){
		nottested();
		return;
	}

	dir  = FPCreateDir(Conn,vol, DIRDID_ROOT , ndir);
	if (!dir) {
		nottested();
		goto fin;
	}
	dir1  = FPCreateDir(Conn,vol, DIRDID_ROOT , ndir1);
	if (!dir1) {
		nottested();
		goto fin;
	}

	FAIL (htonl(AFPERR_PARAM) != FPEnumerateFull(Conn, vol, 0, 1, 800,  DIRDID_ROOT, "", bitmap, 0))
	FAIL (htonl(AFPERR_PARAM) != FPEnumerateFull(Conn, vol, 1, 1, 2,  DIRDID_ROOT, "", bitmap, bitmap))

	FAIL (FPEnumerateFull(Conn, vol, 1, 5, 800,  DIRDID_ROOT, "", bitmap, bitmap))
	FAIL (htonl(AFPERR_NOOBJ) != FPEnumerateFull(Conn, vol, 1, 0, 800,  DIRDID_ROOT, "", bitmap, bitmap))
	FAIL (FPEnumerateFull(Conn, vol, 2, 5, 800,  DIRDID_ROOT, "", bitmap, bitmap))
	FAIL (htonl(AFPERR_NOOBJ) != FPEnumerateFull(Conn, vol, 4, 1, 800,  DIRDID_ROOT, "", bitmap, bitmap))
	FAIL (htonl(AFPERR_NOOBJ) != FPEnumerateFull(Conn, vol, 5, 1, 800,  DIRDID_ROOT, "", bitmap, bitmap))

	/* get the third */
	isdir = 0;
	ret = FPEnumerateFull(Conn, VolID, 3, 1, 800,  DIRDID_ROOT, "", bitmap, 0);
	if (htonl(AFPERR_NOOBJ) == ret) {
		isdir = 1;
		ret = FPEnumerateFull(Conn, VolID, 3, 1, 800,  DIRDID_ROOT, "", 0,bitmap);
	}	
	if (ret) {
		failed();
		goto fin;
	}
	filedir.isdir = isdir;
	if (isdir) {
		afp_filedir_unpack(&filedir, dsi->data +ofs, 0, bitmap);
	}
	else {
		afp_filedir_unpack(&filedir, dsi->data +ofs, bitmap, 0);
	}
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT, filedir.lname))
	FAIL (FPEnumerateFull(Conn, vol, 2, 5, 800,  DIRDID_ROOT, "", bitmap, bitmap))
	FAIL (FPEnumerateFull(Conn, vol, 2, 5, 800,  DIRDID_ROOT, "", bitmap, bitmap))

fin:
	FPDelete(Conn, vol,  DIRDID_ROOT, name); 
	FPDelete(Conn, vol,  DIRDID_ROOT, ndir);
	FPDelete(Conn, vol,  DIRDID_ROOT, ndir1);
	FAIL (htonl(AFPERR_NOOBJ) != FPEnumerateFull(Conn, vol, 1, 1, 800,  DIRDID_ROOT, "", bitmap, bitmap))

}

/* ----------- */
void FPEnumerate_test()
{
    fprintf(stderr,"===================\n");
    fprintf(stderr,"FPEnumerate page 150\n");
    test38();
    test40();
    test41();
    test93();
    test218();
}


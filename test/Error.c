/* ----------------------------------------------
*/
#include "specs.h"

/* ----------------------- */
static unsigned char afp_cmd_with_vol_did[] = {
	AFP_COPYFILE, 			/* 5 */
	AFP_CREATEDIR,			/* 6 */
	AFP_CREATEFILE,			/* 7 */
	AFP_DELETE,				/* 8 */
	AFP_OPENDIR,			/* 25 */
	AFP_OPENFORK,           /* 26 */
	AFP_RENAME,				/* 28 */
	AFP_SETDIRPARAM,		/* 29 */
	AFP_SETFILEPARAM, 		/* 30 */
	AFP_GETFLDRPARAM,		/* 34 */
	AFP_SETFLDRPARAM,		/* 35 */
	AFP_ADDAPPL,			/* 53 */
	AFP_RMVAPPL,			/* 54 */
	AFP_ADDCMT,				/* 56 */
	AFP_RMVCMT,				/* 57 */
	AFP_GETCMT,				/* 58 */
};

STATIC void test36()
{
unsigned int i;
int ofs;
u_int16_t param = VolID;
char *name = "t36 dir";
int  did;
u_int16_t vol = VolID;
DSI *dsi;
unsigned char cmd;
unsigned int ret;

	dsi = &Conn->dsi;

    fprintf(stderr,"===================\n");
    fprintf(stderr,"Error:test36: no folder error (ERR_NOOBJ)\n");
	memset(dsi->commands, 0, sizeof(dsi->commands));
	did  = FPCreateDir(Conn,vol, DIRDID_ROOT , name);
	if (!did) {
		nottested();
		return;
	}
	if (FPDelete(Conn, vol,  DIRDID_ROOT, name)) {
		nottested();
		return;
	}

	for (i = 0 ;i < sizeof(afp_cmd_with_vol_did);i++) {
		memset(dsi->commands, 0, DSI_CMDSIZ);
		dsi->header.dsi_flags = DSIFL_REQUEST;     
		dsi->header.dsi_command = DSIFUNC_CMD;
		dsi->header.dsi_requestID = htons(dsi_clientID(dsi));

		ofs = 0;
		cmd = afp_cmd_with_vol_did[i];
		dsi->commands[ofs++] = cmd;
		dsi->commands[ofs++] = 0;

		memcpy(dsi->commands +ofs, &param, sizeof(param));
		ofs += sizeof(param);

		memcpy(dsi->commands +ofs, &did, sizeof(did));  /* directory did */
		ofs += sizeof(did);
		
		dsi->datalen = ofs;
		dsi->header.dsi_len = htonl(dsi->datalen);
		dsi->header.dsi_code = 0; // htonl(err);
 
   		my_dsi_stream_send(dsi, dsi->commands, dsi->datalen);
		my_dsi_receive(dsi);
		ret = dsi->header.dsi_code;
    	if (ntohl(AFPERR_NOOBJ) != ret) {
			fprintf(stderr,"\tFAILED command %3i %s\t result %d %s\n", cmd, AfpNum2name(cmd),ntohl(ret), afp_error(ret));
			failed_nomsg();
    	}
    }
}

/* ---------------------- 
afp_openfork
afp_createfile
afp_setfilparams
afp_copyfile
afp_createid
afp_exchangefiles
dirlookup
afp_setdirparams
afp_createdir
afp_opendir
afp_getdiracl
afp_setdiracl
afp_openvol
afp_getfildirparams
afp_setfildirparams
afp_delete
afp_moveandrename
afp_enumerate
*/

static void cname_test(char *name)
{
int  ofs =  3 * sizeof( u_int16_t );
struct afp_filedir_parms filedir;
u_int16_t bitmap = (1 <<  DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID) |
	               (1 << DIRPBIT_ACCESS);
u_int16_t vol = VolID;
DSI *dsi;

	dsi = &Conn->dsi;

	FAIL (FPGetFileDirParams(Conn, vol, DIRDID_ROOT, name, 0,bitmap )) 
	filedir.isdir = 1;
	afp_filedir_unpack(&filedir, dsi->data +ofs, 0, bitmap);

	if (filedir.pdid != 2) {
		fprintf(stderr,"\tFAILED %x should be %x\n",filedir.pdid, 2 );
		failed_nomsg();
	}
	if (strcmp(filedir.lname, name)) {
		fprintf(stderr,"\tFAILED %s should be %s\n",filedir.lname, name );
		failed_nomsg();
	}
	FAIL (FPEnumerate(Conn, vol,  DIRDID_ROOT , "", 0, bitmap)) 
	if (FPGetFileDirParams(Conn, vol, DIRDID_ROOT, name, 0,bitmap )) {
		failed();
		return;
	}
	filedir.isdir = 1;
	afp_filedir_unpack(&filedir, dsi->data +ofs, 0, bitmap);

	if (filedir.pdid != 2) {
		fprintf(stderr,"\tFAILED %x should be %x\n",filedir.pdid, 2 );
		failed_nomsg();
	}
	if (strcmp(filedir.lname, name)) {
		fprintf(stderr,"\tFAILED %s should be %s\n",filedir.lname, name );
		failed_nomsg();
	}

	FAIL (ntohl(AFPERR_NOITEM) != FPGetComment(Conn, vol,  DIRDID_ROOT , name)) 
	FAIL (FPCloseVol(Conn,vol))
	vol  = FPOpenVol(Conn, Vol);
	if (vol == 0xffff) {
		nottested();
		return;
	}
	if (ntohl(AFPERR_NOITEM) != FPGetComment(Conn, vol,  DIRDID_ROOT , name)) {
		failed();
	}
	FAIL (FPEnumerate(Conn, vol,  DIRDID_ROOT , "", 0, bitmap)) 

	if (ntohl(AFPERR_NOITEM) != FPGetComment(Conn, vol,  DIRDID_ROOT , name)) {
		failed();
	}
}

/* ------------------------- */
STATIC void test95()
{
int dir;
char *name  = "t95 exchange file";
char *name1 = "t95 new file name";
char *name2 = "t95 dir";
char *pname = "t95 --";
int pdir;
int ret;
u_int16_t vol = VolID;

	if (!Conn2) 
		return;

    fprintf(stderr,"===================\n");
    fprintf(stderr,"test95: exchange files\n");

	if (!(pdir = no_access_folder(vol, DIRDID_ROOT, pname))) {
		return;
	}

	if (!(dir = FPCreateDir(Conn,vol, DIRDID_ROOT , name2))) {
		nottested();
		return;
	}

	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name2))

	/* sdid bad */
	FAIL (ntohl(AFPERR_NOOBJ) != FPExchangeFile(Conn, vol, dir, DIRDID_ROOT, name, name1)) 

	/* name bad */
	ret = FPExchangeFile(Conn, vol, DIRDID_ROOT, DIRDID_ROOT, name, name1);
	if (not_valid(ret, /* MAC */AFPERR_NOOBJ, AFPERR_NOID)) {
		failed();
	}
	/* a dir */
	ret = FPExchangeFile(Conn, vol, DIRDID_ROOT, DIRDID_ROOT, "", name1);
	if (not_valid(ret, /* MAC */AFPERR_NOOBJ, AFPERR_BADTYPE)) {
		failed();
	}

	/* cname unchdirable */
	ret = FPExchangeFile(Conn, vol, DIRDID_ROOT, DIRDID_ROOT, "t95 --/2.txt", name1);
	if (not_valid(ret, /* MAC */AFPERR_NOOBJ, AFPERR_ACCESS)) {
		failed();
	}

	/* FIXME second time once bar is in the cache */
	FAIL (ntohl(AFPERR_ACCESS) != FPCopyFile(Conn, vol, DIRDID_ROOT, vol, DIRDID_ROOT, "t95 --/2.txt", name1)) 

	FAIL (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name))
#if 0
	if (!(dir1 = FPCreateDir(Conn,vol, DIRDID_ROOT , name2))) {fprintf(stderr,"\tFAILED\n");}

	fork = FPOpenFork(Conn, vol, OPENFORK_DATA , bitmap ,DIRDID_ROOT, name,OPENACC_WR | OPENACC_RD);
	if (fork) {
		if (ntohl(AFPERR_DENYCONF) != FPCopyFile(Conn, vol, DIRDID_ROOT, vol, DIRDID_ROOT, name, name1)) {
			fprintf(stderr,"\tFAILED\n");
		}
		FPCloseFork(Conn,fork);
	}	
#endif
	/* sdid bad */

	FAIL (ntohl(AFPERR_NOOBJ) != FPExchangeFile(Conn, vol, DIRDID_ROOT,dir, name, name1)) 

	/* name bad */
	ret = FPExchangeFile(Conn, vol, DIRDID_ROOT, DIRDID_ROOT, name, name1);
	if (not_valid(ret, /* MAC */AFPERR_NOOBJ, AFPERR_NOID)) {
		failed();
	}
	/* same object */
	FAIL (ntohl(AFPERR_SAMEOBJ) != FPExchangeFile(Conn, vol, DIRDID_ROOT, DIRDID_ROOT, name, name))

	/* a dir */
	FAIL (ntohl(AFPERR_BADTYPE) != FPExchangeFile(Conn, vol, DIRDID_ROOT, DIRDID_ROOT, name, ""))

	FAIL (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name1))

	FAIL (FPExchangeFile(Conn, vol, DIRDID_ROOT, DIRDID_ROOT, name, name1))

	delete_folder(vol, DIRDID_ROOT, pname);
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT, name))
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT, name1))
}

/* ----------------- */
STATIC void test99()
{
int  dir = 0;
char *name = "t99 dir no access";
u_int16_t vol = VolID;

	if (!Conn2)
		return;
		
    fprintf(stderr,"===================\n");
    fprintf(stderr,"Error:t99: test folder without access right\n");

	if (!(dir = no_access_folder(vol, DIRDID_ROOT, name))) {
		return;
	}
    
    cname_test(name);
#if 0
    cname_test("essai permission");
#endif    
	delete_folder(vol, DIRDID_ROOT, name);
}

/* --------------------- */
STATIC void test100()
{
int dir;
char *name = "t100 no obj error";
char *name1 = "t100 no obj error/none";
struct afp_filedir_parms filedir;
u_int16_t bitmap = (1<<DIRPBIT_ATTR);
unsigned int ret;
u_int16_t vol = VolID;
DSI *dsi;

	dsi = &Conn->dsi;

    fprintf(stderr,"===================\n");
    fprintf(stderr,"Error:t100: no obj cname error (AFPERR_NOOBJ)\n");

	FAIL (ntohl(AFPERR_NOOBJ) != FPAddComment(Conn, vol,  DIRDID_ROOT , name1,"essai")) 
	FAIL (ntohl(AFPERR_NOOBJ) != FPGetComment(Conn, vol,  DIRDID_ROOT , name1)) 
	FAIL (ntohl(AFPERR_NOOBJ) != FPRemoveComment(Conn, vol,  DIRDID_ROOT , name1)) 

	filedir.isdir = 1;
	filedir.attr = ATTRBIT_NODELETE | ATTRBIT_SETCLR ;
 	FAIL (ntohl(AFPERR_NOOBJ) != FPSetDirParms(Conn, vol, DIRDID_ROOT , name1, bitmap, &filedir)) 

	if ((dir = FPCreateDir(Conn,vol, DIRDID_ROOT , name1))) {
		failed();
		FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name1))
	}
	else if (ntohl(AFPERR_NOOBJ) != dsi->header.dsi_code) {
		failed();
	}
	    
	dir = FPOpenDir(Conn,vol, DIRDID_ROOT , name1);
    if (dir || ntohl(AFPERR_NOOBJ) != dsi->header.dsi_code) 
		failed();

	if (ntohl(AFPERR_NODIR) != FPEnumerate(Conn, vol,  DIRDID_ROOT , name1, 
	     (1<<FILPBIT_LNAME) | (1<<FILPBIT_FNUM ) | (1<<FILPBIT_ATTR) | (1<<FILPBIT_FINFO)|
	     (1<<FILPBIT_CDATE) | (1<< FILPBIT_PDID)
	      ,
		 (1<< DIRPBIT_ATTR) |
		 (1<< DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID)|(1<< DIRPBIT_ACCESS)
		)) {
		failed();
	}

	if (ntohl(AFPERR_NOOBJ) != FPCopyFile(Conn, vol, DIRDID_ROOT, vol, DIRDID_ROOT, name1, name)) {
		failed();
	}

	/* FIXME afp_errno in file.c */
	if ((get_vol_attrib(vol) & VOLPBIT_ATTR_FILEID) ) {
		ret = FPCreateID(Conn,vol, DIRDID_ROOT, name1);
		if (ret != ntohl(AFPERR_NOOBJ)) {
			failed();
		}
	}
	/* FIXME ? */
	if (ntohl(AFPERR_NOOBJ) != FPExchangeFile(Conn, vol, DIRDID_ROOT, DIRDID_ROOT, name1, name)) {
		failed();
	}

 	FAIL (ntohl(AFPERR_NOOBJ) != FPSetFileParams(Conn, vol, DIRDID_ROOT , name1, bitmap, &filedir)) 
 	FAIL (ntohl(AFPERR_NOOBJ) != FPSetFilDirParam(Conn, vol, DIRDID_ROOT , name1, bitmap, &filedir)) 

	FAIL (ntohl(AFPERR_NOOBJ) != FPRename(Conn, vol, DIRDID_ROOT, name1, name)) 

	FAIL (ntohl(AFPERR_NOOBJ) != FPDelete(Conn, vol,  DIRDID_ROOT , name1))

	FAIL (ntohl(AFPERR_NOOBJ) != FPMoveAndRename(Conn, vol, DIRDID_ROOT, DIRDID_ROOT, name1, name)) 
}

/* --------------------- */
STATIC void test101()
{
int dir;
char *name = "t101 no obj error";
char *ndir = "t101 no";
char *name1 = "t101 no/none";
struct afp_filedir_parms filedir;
u_int16_t bitmap = (1<<DIRPBIT_ATTR);
u_int16_t vol = VolID;
unsigned int ret;
DSI *dsi;

	dsi = &Conn->dsi;

	if (!Conn2)
		return;
		
    fprintf(stderr,"===================\n");
    fprintf(stderr,"Error:t101: access error cname \n");

	if (!(dir = no_access_folder(vol, DIRDID_ROOT, ndir))) {
		return;
	}
	FAIL (ntohl(AFPERR_ACCESS) != FPAddComment(Conn, vol,  DIRDID_ROOT , name1,"essai")) 

	ret = FPGetComment(Conn, vol,  DIRDID_ROOT , name1);
	if (not_valid(ret, /* MAC */AFPERR_NOOBJ, AFPERR_ACCESS)) {
		failed();
	}

	FAIL (ntohl(AFPERR_ACCESS) != FPRemoveComment(Conn, vol,  DIRDID_ROOT , name1)) 
	filedir.isdir = 1;
	filedir.attr = ATTRBIT_NODELETE | ATTRBIT_SETCLR ;
 	FAIL (ntohl(AFPERR_ACCESS) != FPSetDirParms(Conn, vol, DIRDID_ROOT , name1, bitmap, &filedir)) 

	if ((dir = FPCreateDir(Conn,vol, DIRDID_ROOT , name1))) {
		failed();
		FPDelete(Conn, vol,  DIRDID_ROOT , name1);
	}
	else if (ntohl(AFPERR_ACCESS) != dsi->header.dsi_code) {
		failed();
	}
	    
	dir = FPOpenDir(Conn,vol, DIRDID_ROOT , name1);
    if (dir || ntohl(AFPERR_ACCESS) != dsi->header.dsi_code) {
		failed();
	}

	ret = FPEnumerate(Conn, vol,  DIRDID_ROOT , name1, 
	     (1<<FILPBIT_LNAME) | (1<<FILPBIT_FNUM ) | (1<<FILPBIT_ATTR) | (1<<FILPBIT_FINFO)|
	     (1<<FILPBIT_CDATE) | (1<< FILPBIT_PDID)
	      ,
		 (1<< DIRPBIT_ATTR) |
		 (1<< DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID)|(1<< DIRPBIT_ACCESS)
		);
	if (not_valid(ret, /* MAC */AFPERR_NODIR, AFPERR_ACCESS)) {
		failed();
	}

	FAIL (ntohl(AFPERR_ACCESS) != FPCopyFile(Conn, vol, DIRDID_ROOT, vol, DIRDID_ROOT, name1, name)) 

	if ((get_vol_attrib(vol) & VOLPBIT_ATTR_FILEID) ) {
		ret = FPCreateID(Conn,vol, DIRDID_ROOT, name1);
		if (ret != ntohl(AFPERR_ACCESS)) {
			failed();
		}
	}

	ret = FPExchangeFile(Conn, vol, DIRDID_ROOT, DIRDID_ROOT, name1, name);
	if (not_valid(ret, /* MAC */AFPERR_NOOBJ, AFPERR_ACCESS)) {
		failed();
	}

 	FAIL (ntohl(AFPERR_ACCESS) != FPSetFileParams(Conn, vol, DIRDID_ROOT , name1, bitmap, &filedir)) 
	FAIL (ntohl(AFPERR_ACCESS) != FPRename(Conn, vol, DIRDID_ROOT, name1, name)) 
	FAIL (ntohl(AFPERR_ACCESS) != FPDelete(Conn, vol,  DIRDID_ROOT , name1))

	FAIL (ntohl(AFPERR_ACCESS) != FPMoveAndRename(Conn, vol, DIRDID_ROOT, DIRDID_ROOT, name1, name)) 
	delete_folder(vol, DIRDID_ROOT, ndir);
}

/* --------------------- */
static void test_comment(u_int16_t vol, int dir, char *name)
{
int ret;

	ret = FPAddComment(Conn, vol,  dir, name, "essai");
	if (not_valid(ret, /* MAC */0, AFPERR_ACCESS)) {
		failed();
	}

	ret = FPRemoveComment(Conn, vol,  dir , name);
	if (not_valid(ret, /* MAC */0, AFPERR_ACCESS)) {
		failed();
	}

	if (!ret) {
		FPAddComment(Conn, vol,  dir , name,"essai");
	}
}

/* -------------- */
STATIC void test102()
{
int dir;
char *name = "t102 access error";
char *name1 = "t102 dir --";
int ret;
struct afp_filedir_parms filedir;
u_int16_t bitmap = (1<<DIRPBIT_ATTR);
u_int16_t vol = VolID;
DSI *dsi;

	dsi = &Conn->dsi;

	if (!Conn2)
		return;

    fprintf(stderr,"===================\n");
    fprintf(stderr,"Error:t102: access error but not cname \n");

	if (!(dir = no_access_folder(vol, DIRDID_ROOT, name1))) {
		return;
	}

	test_comment(vol, DIRDID_ROOT, name1);
	ret = FPGetComment(Conn, vol,DIRDID_ROOT, name1);
	if (not_valid(ret, /* MAC */0, AFPERR_NOITEM)) {
		failed();
	}
	
	filedir.isdir = 1;
	filedir.attr = ATTRBIT_NODELETE | ATTRBIT_SETCLR ;
 	FAIL (ntohl(AFPERR_ACCESS) != FPSetDirParms(Conn, vol, DIRDID_ROOT , name1, bitmap, &filedir)) 

	if ((dir = FPCreateDir(Conn,vol, DIRDID_ROOT , name1))) {
		failed();
		FPDelete(Conn, vol,  DIRDID_ROOT , name1);
	}
	else if (ntohl(AFPERR_EXIST) != dsi->header.dsi_code) {
		failed();
	}
	    
	dir = FPOpenDir(Conn,vol, DIRDID_ROOT , name1);
    if (dir) {
    	ret = 0;
    }
    else {
    	ret = dsi->header.dsi_code;
    }
	if (not_valid(ret, /* MAC */0, AFPERR_ACCESS)) {
		failed();
	}

	if (ntohl(AFPERR_ACCESS) != FPEnumerate(Conn, vol,  DIRDID_ROOT , name1, 
	     (1<<FILPBIT_LNAME) | (1<<FILPBIT_FNUM ) | (1<<FILPBIT_ATTR) | (1<<FILPBIT_FINFO)|
	     (1<<FILPBIT_CDATE) | (1<< FILPBIT_PDID)
	      ,
		 (1<< DIRPBIT_ATTR) |
		 (1<< DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID)|(1<< DIRPBIT_ACCESS)
		)) {
		failed();
	}

	FAIL (ntohl(AFPERR_BADTYPE) != FPCopyFile(Conn, vol, DIRDID_ROOT, vol, DIRDID_ROOT, name1, name)) 
	if ((get_vol_attrib(vol) & VOLPBIT_ATTR_FILEID) ) {
		FAIL (ntohl(AFPERR_BADTYPE) != FPCreateID(Conn,vol, DIRDID_ROOT, name1)) 
	}

	ret = FPExchangeFile(Conn, vol, DIRDID_ROOT, DIRDID_ROOT, name1, name);
	if (not_valid(ret, /* MAC */AFPERR_NOOBJ, AFPERR_BADTYPE)) {
		failed();
	}

 	FAIL (ntohl(AFPERR_BADTYPE) != FPSetFileParams(Conn, vol, DIRDID_ROOT , name1, bitmap, &filedir))

	if (FPRename(Conn, vol, DIRDID_ROOT, name1, name)) {
		failed();
	}
	else {
		FPRename(Conn, vol, DIRDID_ROOT, name, name1);
	}

	if (FPMoveAndRename(Conn, vol, DIRDID_ROOT, DIRDID_ROOT, name1, name)) {
		failed();
	}
	else {
		FPMoveAndRename(Conn, vol, DIRDID_ROOT, DIRDID_ROOT, name, name1);	
	}
	ret = FPDelete(Conn, vol,  DIRDID_ROOT , name1);
	if (not_valid(ret,0, AFPERR_ACCESS) ) { 
		failed();
	}
	if (ret)
		delete_folder(vol, DIRDID_ROOT, name1);
}

/* --------------------- */
STATIC void test103()
{
int dir;
char *name = "t103 did access error";
char *name1 = "t130 dir --";
int ret;
int  ofs =  3 * sizeof( u_int16_t );
struct afp_filedir_parms filedir;
u_int16_t bitmap = (1<< DIRPBIT_DID);
u_int16_t vol = VolID;
DSI *dsi;

	dsi = &Conn->dsi;

	if (!Conn2)
		return;

    fprintf(stderr,"===================\n");
    fprintf(stderr,"t103: did access error \n");

	if (!(dir = no_access_folder(vol, DIRDID_ROOT, name1))) {
		return;
	}

	FAIL (FPGetFileDirParams(Conn, vol, DIRDID_ROOT, name1, 0, bitmap)) 

	filedir.isdir = 1;
	afp_filedir_unpack(&filedir, dsi->data +ofs, 0, bitmap);
	dir = filedir.did;

	test_comment(vol, dir, "");
	ret = FPGetComment(Conn, vol,  dir , "");
	if (not_valid(ret, /* MAC */AFPERR_ACCESS, AFPERR_NOITEM)) {
		failed();
	}

	filedir.isdir = 1;
	filedir.attr = ATTRBIT_NODELETE | ATTRBIT_SETCLR ;
	bitmap = (1<< DIRPBIT_ATTR);
 	FAIL (ntohl(AFPERR_ACCESS) != FPSetDirParms(Conn, vol, dir , "", bitmap, &filedir)) 

	if (ntohl(AFPERR_ACCESS) != FPEnumerate(Conn, vol,  dir , "", 
	     (1<<FILPBIT_LNAME) | (1<<FILPBIT_FNUM ) | (1<<FILPBIT_ATTR) | (1<<FILPBIT_FINFO)|
	     (1<<FILPBIT_CDATE) | (1<< FILPBIT_PDID)
	      ,
		 (1<< DIRPBIT_ATTR) |
		 (1<< DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID)|(1<< DIRPBIT_ACCESS)
		)) {
		failed();
	}

	ret = FPCopyFile(Conn, vol, dir, vol, DIRDID_ROOT, "", name);
	if (not_valid(ret, AFPERR_PARAM, AFPERR_BADTYPE) ) { 
		failed();
	}

	if ((get_vol_attrib(vol) & VOLPBIT_ATTR_FILEID) ) {
		FAIL (ntohl(AFPERR_BADTYPE) != FPCreateID(Conn,vol, dir, "")) 
	}

	ret = FPExchangeFile(Conn, vol, dir, DIRDID_ROOT, "", name);
	if (not_valid(ret, AFPERR_NOOBJ, AFPERR_BADTYPE) ) { 
		failed();
	}

	ret = FPSetFileParams(Conn, vol, dir , "", bitmap, &filedir);
	if (not_valid(ret, AFPERR_PARAM, AFPERR_BADTYPE) ) { 
		failed();
	}

	if (FPRename(Conn, vol, dir, "", name)) {
		failed();
	}
	else {
		if (FPGetFileDirParams(Conn, vol, DIRDID_ROOT, name, 0, bitmap)) {
			failed();
		}
		FPRename(Conn, vol, DIRDID_ROOT, name, name1);
	}

	if (FPMoveAndRename(Conn, vol, dir, DIRDID_ROOT, "", name)) {
		failed();
	}
	else {
		if (FPGetFileDirParams(Conn, vol, DIRDID_ROOT, name, 0, bitmap)) {
			failed();
		}
		FPMoveAndRename(Conn, vol, DIRDID_ROOT, DIRDID_ROOT, name, name1);	
	}
	
	ret = FPDelete(Conn, vol,  dir , "");
	if (not_valid(ret,0, AFPERR_ACCESS) ) { 
		failed();
	}
	if (ret)
		delete_folder(vol, DIRDID_ROOT, name1);
}

/* --------------------- */
STATIC void test105()
{
int dir;
unsigned int err;
char *name = "t105 bad did";
char *name1 = "t105 no obj error/none";
struct afp_filedir_parms filedir;
u_int16_t bitmap = (1<<DIRPBIT_ATTR);
u_int16_t vol = VolID;
DSI *dsi;
int ret;

	dsi = &Conn->dsi;

    fprintf(stderr,"===================\n");
    fprintf(stderr,"Error:t105: bad DID in call \n");

    dir = 0;
    err = ntohl(AFPERR_PARAM);
	FAIL (err != FPAddComment(Conn, vol, dir, name1,"essai")) 
	FAIL (err != FPGetComment(Conn, vol, dir , name1)) 
	FAIL (err != FPRemoveComment(Conn, vol, dir  , name1)) 

	filedir.isdir = 1;
	filedir.attr = ATTRBIT_NODELETE | ATTRBIT_SETCLR ;
 	FAIL (err != FPSetDirParms(Conn, vol, dir , name1, bitmap, &filedir)) 

	if ((dir = FPCreateDir(Conn,vol, dir , name1))) {
		failed();
		FPDelete(Conn, vol,  dir , name1);
	}
	else if (err != dsi->header.dsi_code) {
		failed();
	}
	    
	dir = FPOpenDir(Conn,vol, dir , name1);
    if (dir || err != dsi->header.dsi_code) {
		failed();
	}

	if (err  != FPEnumerate(Conn, vol,  dir , name1, 
	     (1<<FILPBIT_LNAME) | (1<<FILPBIT_FNUM ) | (1<<FILPBIT_ATTR) | (1<<FILPBIT_FINFO)|
	     (1<<FILPBIT_CDATE) | (1<< FILPBIT_PDID)
	      ,
		 (1<< DIRPBIT_ATTR) |
		 (1<< DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID)|(1<< DIRPBIT_ACCESS)
		)) {
		failed();
	}

	FAIL (err != FPCopyFile(Conn, vol, dir, vol, DIRDID_ROOT, name1, name)) 

	/* FIXME afp_errno in file.c */
	if ((get_vol_attrib(vol) & VOLPBIT_ATTR_FILEID) ) {
		if (err != FPCreateID(Conn,vol, dir, name1)) {
			failed();
		}
	}
	ret = FPExchangeFile(Conn, vol, dir, DIRDID_ROOT, name1, name);
	if (not_valid(ret, /* MAC */AFPERR_NOOBJ, AFPERR_PARAM)) {
		failed();
	}

 	FAIL (err != FPSetFileParams(Conn, vol, dir , name1, bitmap, &filedir)) 
	FAIL (err != FPRename(Conn, vol, dir, name1, name)) 
	FAIL (err != FPDelete(Conn, vol,  dir , name1)) 
	FAIL (err != FPMoveAndRename(Conn, vol, dir, DIRDID_ROOT, name1, name)) 
}

/* -------------------------- */
STATIC void test170()
{
u_int16_t bitmap = 0;
char *name = "test170.txt";
char *name1 = "newtest170.txt";
int  ofs =  3 * sizeof( u_int16_t );
struct afp_filedir_parms filedir;
int fork;
int dir;
unsigned int ret;
u_int16_t vol = VolID;
DSI *dsi;

	dsi = &Conn->dsi;

    fprintf(stderr,"===================\n");
    fprintf(stderr,"Error:test170: cname error did=1 name=\"\"\n");

    /* ---- fork.c ---- */
	fork = FPOpenFork(Conn, vol, OPENFORK_DATA , bitmap ,DIRDID_ROOT_PARENT, "",OPENACC_WR | OPENACC_RD);
	if (fork || htonl(AFPERR_PARAM) != dsi->header.dsi_code) {
		failed();
		if (fork) FPCloseFork(Conn,fork);
	}

    /* ---- file.c ---- */
	FAIL (htonl(AFPERR_PARAM) != FPCreateFile(Conn, vol,  0, DIRDID_ROOT_PARENT , "")) 
	
	bitmap = (1<<FILPBIT_MDATE);

	if (FPGetFileDirParams(Conn, vol,  DIRDID_ROOT , "", 0, bitmap)) {
		failed();
	}
	else {
		filedir.isdir = 1;
		afp_filedir_unpack(&filedir, dsi->data +ofs, 0, bitmap);
		filedir.isdir = 0;
 		FAIL (htonl(AFPERR_PARAM) != FPSetFileParams(Conn, vol, DIRDID_ROOT_PARENT , "", bitmap, &filedir))
	}
	/* -------------------- */
	FAIL (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name)) 
	
	if (htonl(AFPERR_PARAM) != FPCopyFile(Conn, vol, DIRDID_ROOT_PARENT, vol, DIRDID_ROOT, "", name1)) {
		failed();
		FPDelete(Conn, vol,  DIRDID_ROOT , name1);
	}

	FAIL (htonl(AFPERR_NOOBJ) != FPCopyFile(Conn, vol, DIRDID_ROOT, vol, DIRDID_ROOT_PARENT, name, "")) 

	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name))
	
	/* -------------------- */
	if ((get_vol_attrib(vol) & VOLPBIT_ATTR_FILEID) ) {
		ret = FPCreateID(Conn,vol, DIRDID_ROOT_PARENT, "");
		FAIL (htonl(AFPERR_NOOBJ) != ret && htonl(AFPERR_PARAM) != ret ) 
	}

	/* -------------------- */
	FAIL (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name)) 
	FAIL (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name1))
	FAIL (ntohl(AFPERR_NOOBJ) != FPExchangeFile(Conn, vol, DIRDID_ROOT_PARENT,dir, "", name1)) 
	FAIL (ntohl(AFPERR_NOOBJ) != FPExchangeFile(Conn, vol, DIRDID_ROOT, DIRDID_ROOT_PARENT, name, ""))

	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name))
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name1)) 
	
	/* ---- directory.c ---- */
	filedir.isdir = 1;
 	FAIL (ntohl(AFPERR_NOOBJ) != FPSetDirParms(Conn, vol, DIRDID_ROOT_PARENT , "", bitmap, &filedir))
 	/* ---------------- */
	dir  = FPCreateDir(Conn,vol, DIRDID_ROOT_PARENT , "");
	if (dir || ntohl(AFPERR_PARAM) != dsi->header.dsi_code) {
		failed();
	}

 	/* ---------------- */
	dir = FPOpenDir(Conn,vol, DIRDID_ROOT_PARENT , "");
    if (dir || ntohl(AFPERR_PARAM) != dsi->header.dsi_code) {
		failed();
	}
 	/* ---- filedir.c ---- */

	if (ntohl(AFPERR_NOOBJ) != FPGetFileDirParams(Conn, vol, DIRDID_ROOT_PARENT, "", 0, 
	        (1 <<  DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID) | (1<<DIRPBIT_UID) |
	    	(1 << DIRPBIT_GID) |(1 << DIRPBIT_ACCESS))
	) {
		failed();
	}

 	/* ---------------- */
	if (FPGetFileDirParams(Conn, vol,  DIRDID_ROOT , "", 0,bitmap )) {
		failed();
	}
	else {
		filedir.isdir = 1;
		afp_filedir_unpack(&filedir, dsi->data +ofs, 0, bitmap);
 		if (ntohl(AFPERR_NOOBJ) != FPSetFilDirParam(Conn, vol, DIRDID_ROOT_PARENT , "", bitmap, &filedir)) {
			failed();
 		}
 	}
 	/* ---------------- */
	FAIL (ntohl(AFPERR_NOOBJ) != FPRename(Conn, vol, DIRDID_ROOT_PARENT, "", name1)) 
 	/* ---------------- */
	FAIL (ntohl(AFPERR_NOOBJ) != FPDelete(Conn, vol,  DIRDID_ROOT_PARENT , "")) 
 	/* ---------------- */
	FAIL (ntohl(AFPERR_NOOBJ) != FPMoveAndRename(Conn, vol, DIRDID_ROOT_PARENT, DIRDID_ROOT, "", name1))
	FAIL (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name)) 
	FAIL (ntohl(AFPERR_NOOBJ) != FPMoveAndRename(Conn, vol, DIRDID_ROOT, DIRDID_ROOT_PARENT, name, "")) 
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name)) 

	/* ---- enumerate.c ---- */
	if (ntohl(AFPERR_NOOBJ) != FPEnumerate(Conn, vol,  DIRDID_ROOT_PARENT , "", 
	     (1<<FILPBIT_LNAME) | (1<<FILPBIT_FNUM ) | (1<<FILPBIT_ATTR) | (1<<FILPBIT_FINFO)|
	     (1<<FILPBIT_CDATE) | (1<< FILPBIT_PDID)
	      ,
		 (1<< DIRPBIT_ATTR) |
		 (1<< DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID)|(1<< DIRPBIT_ACCESS)
		)) {
		failed();
	}
	/* ---- desktop.c ---- */
	FAIL (ntohl(AFPERR_NOOBJ) != FPAddComment(Conn, vol,  DIRDID_ROOT_PARENT , "", "Comment"))
	FAIL (ntohl(AFPERR_NOOBJ) != FPGetComment(Conn, vol,  DIRDID_ROOT_PARENT , "")) 
	FAIL (ntohl(AFPERR_NOOBJ) != FPRemoveComment(Conn, vol,  DIRDID_ROOT_PARENT , "")) 
}

/* -------------------------- */
STATIC void test171()
{
u_int16_t bitmap = 0;
char *tname = "test171";
char *name = "test171.txt";
char *name1 = "newtest171.txt";
int  ofs =  3 * sizeof( u_int16_t );
struct afp_filedir_parms filedir;
int tdir = DIRDID_ROOT_PARENT;
int fork;
int dir;
unsigned int ret;
u_int16_t vol = VolID;
DSI *dsi;

	dsi = &Conn->dsi;

    fprintf(stderr,"===================\n");
    fprintf(stderr,"Error:test171: cname error did=1 name=bad name\n");

    /* ---- fork.c ---- */
	fork = FPOpenFork(Conn, vol, OPENFORK_DATA , bitmap , tdir, tname,OPENACC_WR | OPENACC_RD);
	if (fork || htonl(AFPERR_NOOBJ) != dsi->header.dsi_code) {
		failed();
		if (fork) FPCloseFork(Conn,fork);
	}

    /* ---- file.c ---- */
	FAIL (htonl(AFPERR_NOOBJ) != FPCreateFile(Conn, vol,  0, tdir, tname)) 
	
	bitmap = (1<<FILPBIT_MDATE);

	if (FPGetFileDirParams(Conn, vol,  DIRDID_ROOT , "", 0, bitmap)) {
		failed();
	}
	else {
		filedir.isdir = 1;
		afp_filedir_unpack(&filedir, dsi->data +ofs, 0, bitmap);
		filedir.isdir = 0;
 		FAIL (htonl(AFPERR_NOOBJ) != FPSetFileParams(Conn, vol, tdir, tname, bitmap, &filedir))
	}
	/* -------------------- */
	FAIL (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name)) 
	if (htonl(AFPERR_NOOBJ) != FPCopyFile(Conn, vol, tdir , vol, DIRDID_ROOT, tname, name1)) {
		failed();
		FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name1))
	}

	FAIL (htonl(AFPERR_NOOBJ) != FPCopyFile(Conn, vol, DIRDID_ROOT, vol, tdir, name, tname)) 
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name)) 
	
	/* -------------------- */
	if ((get_vol_attrib(vol) & VOLPBIT_ATTR_FILEID) ) {
		ret = FPCreateID(Conn,vol, tdir, tname);
		if (htonl(AFPERR_NOOBJ) != ret && htonl(AFPERR_PARAM) != ret ) {
			failed();
		}
	}

	/* -------------------- */
	FAIL (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name)) 
	FAIL (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name1)) 
	FAIL (ntohl(AFPERR_NOOBJ) != FPExchangeFile(Conn, vol, tdir,dir, tname, name1)) 
	FAIL (ntohl(AFPERR_NOOBJ) != FPExchangeFile(Conn, vol, DIRDID_ROOT, tdir, name, tname)) 
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name))
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name1)) 
	
	/* ---- directory.c ---- */
	filedir.isdir = 1;
 	FAIL (ntohl(AFPERR_NOOBJ) != FPSetDirParms(Conn, vol, tdir, tname, bitmap, &filedir))
 	/* ---------------- */
	dir  = FPCreateDir(Conn,vol, tdir, tname);
	if (dir || ntohl(AFPERR_NOOBJ) != dsi->header.dsi_code) {
		failed();
	}

 	/* ---------------- */
	dir = FPOpenDir(Conn,vol, tdir, tname);
    FAIL (dir || ntohl(AFPERR_NOOBJ) != dsi->header.dsi_code) 
 	/* ---- filedir.c ---- */

	if (ntohl(AFPERR_NOOBJ) != FPGetFileDirParams(Conn, vol, tdir, tname, 0, 
	        (1 <<  DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID) | (1<<DIRPBIT_UID) |
	    	(1 << DIRPBIT_GID) |(1 << DIRPBIT_ACCESS))
	) {
		failed();
	}

 	/* ---------------- */
	if (FPGetFileDirParams(Conn, vol,  DIRDID_ROOT , "", 0,bitmap )) {
		failed();
	}
	else {
		filedir.isdir = 1;
		afp_filedir_unpack(&filedir, dsi->data +ofs, 0, bitmap);
 		FAIL (ntohl(AFPERR_NOOBJ) != FPSetFilDirParam(Conn, vol, tdir, tname, bitmap, &filedir))
 	}
 	/* ---------------- */
	FAIL (ntohl(AFPERR_NOOBJ) != FPRename(Conn, vol, tdir, tname, name1)) 
 	/* ---------------- */
	FAIL (ntohl(AFPERR_NOOBJ) != FPDelete(Conn, vol,  tdir, tname))
 	/* ---------------- */
	FAIL (ntohl(AFPERR_NOOBJ) != FPMoveAndRename(Conn, vol, tdir, DIRDID_ROOT, tname, name1))
	FAIL (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name)) 
	FAIL (ntohl(AFPERR_NOOBJ) != FPMoveAndRename(Conn, vol, DIRDID_ROOT, tdir, name, tname))
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name))

	/* ---- enumerate.c ---- */
	if (ntohl(AFPERR_NODIR) != FPEnumerate(Conn, vol,  tdir, tname, 
	     (1<<FILPBIT_LNAME) | (1<<FILPBIT_FNUM ) | (1<<FILPBIT_ATTR) | (1<<FILPBIT_FINFO)|
	     (1<<FILPBIT_CDATE) | (1<< FILPBIT_PDID)
	      ,
		 (1<< DIRPBIT_ATTR) |
		 (1<< DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID)|(1<< DIRPBIT_ACCESS)
		)) {
		failed();
	}
	/* ---- desktop.c ---- */
	FAIL (ntohl(AFPERR_NOOBJ) != FPAddComment(Conn, vol, tdir, tname, "Comment")) 
	FAIL (ntohl(AFPERR_NOOBJ) != FPGetComment(Conn, vol, tdir, tname)) 
	FAIL (ntohl(AFPERR_NOOBJ) != FPRemoveComment(Conn, vol, tdir, tname)) 
}

/* -------------------------- */
STATIC void test173()
{
u_int16_t bitmap = 0;
char *tname = "test173";
char *name = "test173.txt";
char *name1 = "newtest173.txt";
int  ofs =  3 * sizeof( u_int16_t );
struct afp_filedir_parms filedir;
int tdir = 0;
int fork;
int dir;
unsigned int ret;
u_int16_t vol = VolID;
DSI *dsi;

	dsi = &Conn->dsi;

    fprintf(stderr,"===================\n");
    fprintf(stderr,"Error:test173: did error did=0 name=test173 name\n");

    /* ---- fork.c ---- */
	fork = FPOpenFork(Conn, vol, OPENFORK_DATA , bitmap , tdir, tname,OPENACC_WR | OPENACC_RD);
	if (fork || htonl(AFPERR_PARAM) != dsi->header.dsi_code) {
		failed();
		if (fork) FPCloseFork(Conn,fork);
	}

    /* ---- file.c ---- */
	FAIL (htonl(AFPERR_PARAM) != FPCreateFile(Conn, vol,  0, tdir, tname))
	
	bitmap = (1<<FILPBIT_MDATE);

	if (FPGetFileDirParams(Conn, vol,  DIRDID_ROOT , "", 0, bitmap)) {
		failed();
	}
	else {
		filedir.isdir = 1;
		afp_filedir_unpack(&filedir, dsi->data +ofs, 0, bitmap);
		filedir.isdir = 0;
 		FAIL (htonl(AFPERR_PARAM) != FPSetFileParams(Conn, vol, tdir, tname, bitmap, &filedir))
	}
	/* -------------------- */
	FAIL (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name)) 
	
	if (htonl(AFPERR_PARAM) != FPCopyFile(Conn, vol, tdir , vol, DIRDID_ROOT, tname, name1)) {
		failed();
		FPDelete(Conn, vol,  DIRDID_ROOT , name1);
	}

	FAIL (htonl(AFPERR_PARAM) != FPCopyFile(Conn, vol, DIRDID_ROOT, vol, tdir, name, tname)) 
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name))
	
	/* -------------------- */
	if ((get_vol_attrib(vol) & VOLPBIT_ATTR_FILEID) ) {
		ret = FPCreateID(Conn,vol, tdir, tname);
		FAIL (htonl(AFPERR_PARAM) != ret) 
	}

	/* -------------------- */
	FAIL (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name)) 
	FAIL (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name1)) 
	ret = FPExchangeFile(Conn, vol, tdir,dir, tname, name1);
	if (htonl(AFPERR_PARAM) != ret && not_valid(ret, /* MAC */AFPERR_NOOBJ, AFPERR_PARAM)) {
		failed();
	}
	FAIL (ntohl(AFPERR_PARAM) != FPExchangeFile(Conn, vol, DIRDID_ROOT, tdir, name, tname)) 
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name))
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name1)) 
	
	/* ---- directory.c ---- */
	filedir.isdir = 1;
 	FAIL (ntohl(AFPERR_PARAM) != FPSetDirParms(Conn, vol, tdir, tname, bitmap, &filedir)) 
 	/* ---------------- */
	dir  = FPCreateDir(Conn,vol, tdir, tname);
	if (dir || ntohl(AFPERR_PARAM) != dsi->header.dsi_code) {
		failed();
	}

 	/* ---------------- */
	dir = FPOpenDir(Conn,vol, tdir, tname);
    if (dir || ntohl(AFPERR_PARAM) != dsi->header.dsi_code) {
		failed();
	}
 	/* ---- filedir.c ---- */

	if (ntohl(AFPERR_PARAM) != FPGetFileDirParams(Conn, vol, tdir, tname, 0, 
	        (1 <<  DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID) | (1<<DIRPBIT_UID) |
	    	(1 << DIRPBIT_GID) |(1 << DIRPBIT_ACCESS))
	) {
		failed();
	}

 	/* ---------------- */
	if (FPGetFileDirParams(Conn, vol,  DIRDID_ROOT , "", 0,bitmap )) {
		failed();
	}
	else {
		filedir.isdir = 1;
		afp_filedir_unpack(&filedir, dsi->data +ofs, 0, bitmap);
 		FAIL (ntohl(AFPERR_PARAM) != FPSetFilDirParam(Conn, vol, tdir, tname, bitmap, &filedir)) 
 	}
 	/* ---------------- */
	FAIL (ntohl(AFPERR_PARAM) != FPRename(Conn, vol, tdir, tname, name1)) 
 	/* ---------------- */
	FAIL (ntohl(AFPERR_PARAM) != FPDelete(Conn, vol,  tdir, tname)) 
 	/* ---------------- */
	FAIL (ntohl(AFPERR_PARAM) != FPMoveAndRename(Conn, vol, tdir, DIRDID_ROOT, tname, name1)) 
	FAIL (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name)) 
	FAIL (ntohl(AFPERR_PARAM) != FPMoveAndRename(Conn, vol, DIRDID_ROOT, tdir, name, tname)) 
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name))
	/* ---- enumerate.c ---- */
	if (ntohl(AFPERR_PARAM) != FPEnumerate(Conn, vol,  tdir, tname, 
	     (1<<FILPBIT_LNAME) | (1<<FILPBIT_FNUM ) | (1<<FILPBIT_ATTR) | (1<<FILPBIT_FINFO)|
	     (1<<FILPBIT_CDATE) | (1<< FILPBIT_PDID)
	      ,
		 (1<< DIRPBIT_ATTR) |
		 (1<< DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID)|(1<< DIRPBIT_ACCESS)
		)) {
		failed();
	}
	/* ---- desktop.c ---- */
	FAIL (ntohl(AFPERR_PARAM) != FPAddComment(Conn, vol, tdir, tname, "Comment"))
	FAIL (ntohl(AFPERR_PARAM) != FPGetComment(Conn, vol, tdir, tname))
	FAIL (ntohl(AFPERR_PARAM) != FPRemoveComment(Conn, vol, tdir, tname))

	/* ---- appl.c ---- */
}

/* -------------------------- */
STATIC void test174()
{
u_int16_t bitmap = 0;
char *tname = "test174";
char *name = "test174.txt";
char *name1 = "newtest174.txt";
int  ofs =  3 * sizeof( u_int16_t );
struct afp_filedir_parms filedir;
int tdir;
int fork;
int dir;
u_int16_t vol2;
unsigned int ret;
u_int16_t vol = VolID;
DSI *dsi = &Conn->dsi;
DSI *dsi2;

	if (!Conn2) 
		return;
	
    fprintf(stderr,"===================\n");
    fprintf(stderr,"Error:test174: did error two users from parent folder did=<deleted> name=test174 name\n");

	tdir  = FPCreateDir(Conn,vol, DIRDID_ROOT, tname);
	if (!tdir) {
		nottested();
		return;
	}

	FAIL (FPGetFileDirParams(Conn, vol,  tdir , "", 0, (1 << DIRPBIT_PDINFO ) | (1 << DIRPBIT_OFFCNT)))

	FAIL (FPGetFileDirParams(Conn, vol,  DIRDID_ROOT , "", 0, (1 << DIRPBIT_PDINFO ) | (1 << DIRPBIT_OFFCNT))) 

	dsi2 = &Conn2->dsi;
	vol2  = FPOpenVol(Conn2, Vol);
	if (vol2 == 0xffff) {
		failed();
	}

	if (FPDelete(Conn2, vol2,  DIRDID_ROOT , tname)) { 
		failed();
		FPDelete(Conn, vol,  tdir , "");
		FPCloseVol(Conn2,vol2);
		return;
	}
	FPCloseVol(Conn2,vol2);

    /* ---- fork.c ---- */
	fork = FPOpenFork(Conn, vol, OPENFORK_DATA , bitmap , tdir, tname,OPENACC_WR | OPENACC_RD);
	if (fork || htonl(AFPERR_NOOBJ) != dsi->header.dsi_code) {
		failed();
		if (fork) FPCloseFork(Conn,fork);
	}

    /* ---- file.c ---- */
	FAIL (htonl(AFPERR_NOOBJ) != FPCreateFile(Conn, vol,  0, tdir, tname)) 
	
	bitmap = (1<<FILPBIT_MDATE);

	if (FPGetFileDirParams(Conn, vol,  DIRDID_ROOT , "", 0, bitmap)) {
		failed();
	}
	else {
		filedir.isdir = 1;
		afp_filedir_unpack(&filedir, dsi->data +ofs, 0, bitmap);
		filedir.isdir = 0;
 		FAIL (htonl(AFPERR_NOOBJ) != FPSetFileParams(Conn, vol, tdir, tname, bitmap, &filedir)) 
	}
	/* -------------------- */
	FAIL (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name)) 
	
	if (htonl(AFPERR_NOOBJ) != FPCopyFile(Conn, vol, tdir , vol, DIRDID_ROOT, tname, name1)) {
		failed();
		FPDelete(Conn, vol,  DIRDID_ROOT , name1);
	}

	FAIL (htonl(AFPERR_NOOBJ) != FPCopyFile(Conn, vol, DIRDID_ROOT, vol, tdir, name, tname)) 

	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name))
	
	/* -------------------- */
	if ((get_vol_attrib(vol) & VOLPBIT_ATTR_FILEID) ) {
		ret = FPCreateID(Conn,vol, tdir, tname);
		if (htonl(AFPERR_NOOBJ) != ret && htonl(AFPERR_PARAM) != ret ) {
			failed();
		}
	}

	/* -------------------- */
	FAIL (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name)) 
	
	FAIL (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name1)) 

	ret = FPExchangeFile(Conn, vol, tdir,dir, tname, name1);
	if (ntohl(AFPERR_NOOBJ) != ret) {
		if (Quirk && ret == htonl(AFPERR_PARAM)) 
			fprintf(stderr,"\tFAILED (IGNORED) not always the same error code!\n");
		else 
			fprintf(stderr,"\tFAILED\n");
	}
	FAIL (ntohl(AFPERR_NOOBJ) != FPExchangeFile(Conn, vol, DIRDID_ROOT, tdir, name, tname)) 
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name))
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name1)) 
	
	/* ---- directory.c ---- */
	filedir.isdir = 1;
 	FAIL (ntohl(AFPERR_NOOBJ) != FPSetDirParms(Conn, vol, tdir, tname, bitmap, &filedir)) 
 	/* ---------------- */
	dir  = FPCreateDir(Conn,vol, tdir, tname);
	if (dir || ntohl(AFPERR_NOOBJ) != dsi->header.dsi_code) {
		failed();
	}

 	/* ---------------- */
	dir = FPOpenDir(Conn,vol, tdir, tname);
    if (dir || ntohl(AFPERR_NOOBJ) != dsi->header.dsi_code) {
		failed();
	}
 	/* ---- filedir.c ---- */

	if (ntohl(AFPERR_NOOBJ) != FPGetFileDirParams(Conn, vol, tdir, tname, 0, 
	        (1 <<  DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID) | (1<<DIRPBIT_UID) |
	    	(1 << DIRPBIT_GID) |(1 << DIRPBIT_ACCESS))
	) {
		failed();
	}

 	/* ---------------- */
	if (FPGetFileDirParams(Conn, vol,  DIRDID_ROOT , "", 0,bitmap )) {
		failed();
	}
	else {
		filedir.isdir = 1;
		afp_filedir_unpack(&filedir, dsi->data +ofs, 0, bitmap);
 		FAIL (ntohl(AFPERR_NOOBJ) != FPSetFilDirParam(Conn, vol, tdir, tname, bitmap, &filedir)) 
 	}
 	/* ---------------- */
	FAIL (ntohl(AFPERR_NOOBJ) != FPRename(Conn, vol, tdir, tname, name1)) 
 	/* ---------------- */
	FAIL (ntohl(AFPERR_NOOBJ) != FPDelete(Conn, vol,  tdir, tname))
 	/* ---------------- */
	FAIL (ntohl(AFPERR_NOOBJ) != FPMoveAndRename(Conn, vol, tdir, DIRDID_ROOT, tname, name1))
	FAIL (FPCreateFile(Conn, vol,  0, DIRDID_ROOT , name))
	FAIL (ntohl(AFPERR_NOOBJ) != FPMoveAndRename(Conn, vol, DIRDID_ROOT, tdir, name, tname))
	FAIL (FPDelete(Conn, vol,  DIRDID_ROOT , name))

	/* ---- enumerate.c ---- */
	if (ntohl(AFPERR_NODIR) != FPEnumerate(Conn, vol,  tdir, tname, 
	     (1<<FILPBIT_LNAME) | (1<<FILPBIT_FNUM ) | (1<<FILPBIT_ATTR) | (1<<FILPBIT_FINFO)|
	     (1<<FILPBIT_CDATE) | (1<< FILPBIT_PDID)
	      ,
		 (1<< DIRPBIT_ATTR) |
		 (1<< DIRPBIT_LNAME) | (1<< DIRPBIT_PDID) | (1<< DIRPBIT_DID)|(1<< DIRPBIT_ACCESS)
		)) {
		failed();
	}
	/* ---- desktop.c ---- */
	FAIL (ntohl(AFPERR_NOOBJ) != FPAddComment(Conn, vol, tdir, tname, "Comment")) 
	FAIL (ntohl(AFPERR_NOOBJ) != FPGetComment(Conn, vol, tdir, tname))
	FAIL (ntohl(AFPERR_NOOBJ) != FPRemoveComment(Conn, vol, tdir, tname))
}

/* ----------- */
void Error_test()
{
    fprintf(stderr,"===================\n");
    fprintf(stderr,"Various errors\n");
	test36();
	test95();
	test99();
	test100();
	test101();
	test102();
	test103();
	test105();
	test170();
	test171();
	test173();
	test174();
}


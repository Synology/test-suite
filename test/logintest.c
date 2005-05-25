/*
 * $Id: logintest.c,v 1.4 2005-05-25 18:03:32 didg Exp $
 * MANIFEST
 */
#include "specs.h"

int Verbose = 0;
int Quirk = 0;

u_int16_t VolID;

CONN *Conn;
CONN *Conn2;

int ExitCode = 0;

DSI *Dsi;

char Data[300000] = "";
/* ------------------------------- */
char    *Server = "localhost";
char    *Server2;
int     Proto = 0;
int     Port = 548;
char    *Password = "";
char    *Vol = "";
char    *User;
char    *User2;
char    *Path;
int     Version = 21;
int     List = 0;
int     Mac = 0;
char    *Test;
static char  *vers = "AFPVersion 2.1";

static void connect_server(CONN *conn)
{
DSI *dsi;

    conn->type = Proto;
    if (!Proto) {
	int sock;
    	dsi = &conn->dsi;
	    sock = OpenClientSocket(Server, Port);
        if ( sock < 0) {
        	nottested();
	    	exit(ExitCode);
        }
     	dsi->protocol = DSI_TCPIP; 
	    dsi->socket = sock;
    }
    else {
	}
}

/* ------------------------- */
static void test3(void)
{
static char *uam = "No User Authent";
int ret;
    fprintf(stderr,"===================\n");
    fprintf(stderr,"test3: Guest login\n");

    if (Version >= 30) {
      	ret = FPopenLoginExt(Conn, vers, uam, "", "");
    }
    else {
      	ret = FPopenLogin(Conn, vers, uam, "", "");
	}
	if (ret) {      
		failed();
		return;
	}
    if (Mac) {
		fprintf(stderr,"DSIGetStatus\n");
		if (DSIGetStatus(Conn)) {
			failed();
		}
	}
	if (FPLogOut(Conn)) {
		failed();
    }   
}

/* ------------------------- */
static void test4(void)
{
CONN conn[50];
int  i;
int  cnt = 0;
int  ret;

    fprintf(stderr,"===================\n");
    fprintf(stderr,"test4: too many connections\n");
    for (i = 0; i < 50; i++) {
    	connect_server(&conn[i]);
        Dsi = &conn[i].dsi;

        fprintf(stderr,"===================\n");

     	fprintf(stderr,"DSIOpenSession number %d\n", i);
	    if ((ret = DSIOpenSession(&conn[i]))) {
	    	if (ret != DSIERR_TOOMANY) {
				failed();
				return ;
	    	}
	    	cnt = i;
	    	break;
	    }
	}
	if (!cnt) {
		nottested();
		return ;
	}
	for (i = 0; i < cnt; i++) {
        Dsi = &conn[i].dsi;
		if (DSICloseSession(&conn[i])) {
			failed();
			return;
		}
		CloseClientSocket(Dsi->socket);
	}
}

/* =============================== */
void usage( char * av0 )
{
    fprintf( stderr, "usage:\t%s [-m] [-n] [-t] [-h host] [-p port] [-s vol] [-u user] [-w password] -f [call]\n", av0 );
    fprintf( stderr,"\t-m\tserver is a Mac\n");
    fprintf( stderr,"\t-h\tserver host name (default localhost)\n");
    fprintf( stderr,"\t-p\tserver port (default 548)\n");
    fprintf( stderr,"\t-u\tuser name (default uid)\n");
    
    fprintf( stderr,"\t-w\tpassword (default none)\n");
    fprintf( stderr,"\t-2\tAFP 2.2 version (default 2.1)\n");
    fprintf( stderr,"\t-3\tAFP 3.0 version\n");
    fprintf( stderr,"\t-4\tAFP 3.1 version\n");
    fprintf( stderr,"\t-5\tAFP 3.2 version\n");
    fprintf( stderr,"\t-v\tverbose\n");

    exit (1);
}

/* ------------------------------- */
int main( ac, av )
int		ac;
char	**av;
{
int cc;
static char *uam = "Cleartxt Passwrd";
unsigned int ret;

    while (( cc = getopt( ac, av, "v2345h:p:u:w:m" )) != EOF ) {
        switch ( cc ) {
        case '2':
			vers = "AFP2.2";
			Version = 22;
			break;        
        case '3':
			vers = "AFPX03";
			Version = 30;
			break;
        case '4':
			vers = "AFP3.1";
			Version = 31;
			break;
        case '5':
			vers = "AFP3.2";
			Version = 32;
			break;
		case 'm':
			Mac = 1;
			break;
        case 'h':
            Server = strdup(optarg);
            break;
        case 'u':
            User = strdup(optarg);
            break;
        case 'w':
            Password = strdup(optarg);
            break;
        case 'p' :
            Port = atoi( optarg );
            if (Port <= 0) {
                fprintf(stderr, "Bad port.\n");
                exit(1);
            }
            break;
		case 'v':
			Verbose = 1;
			break;
        default :
            usage( av[ 0 ] );
        }
    }
	/************************************
	 *                                  *
	 * Connection user 1                *
	 *                                  *
	 ************************************/

    if ((Conn = (CONN *)calloc(1, sizeof(CONN))) == NULL) {
    	return 1;
    }
    connect_server(Conn);
	/* dsi with no open session */    
    Dsi = &Conn->dsi;

    fprintf(stderr,"===================\n");
	fprintf(stderr, "DSIGetStatus\n");
	if (DSIGetStatus(Conn)) {
		failed();
		return ExitCode;
	}
	CloseClientSocket(Dsi->socket);

	/* ------------------------ */	
    connect_server(Conn);
    Dsi = &Conn->dsi;

    fprintf(stderr,"===================\n");

	fprintf(stderr,"DSIOpenSession\n");
	if (DSIOpenSession(Conn)) {
		failed();
		return ExitCode;
	}
	if (Mac) {
		fprintf(stderr,"DSIGetStatus\n");
		if (DSIGetStatus(Conn)) {
			failed();
			return ExitCode;
		}
	}
	fprintf(stderr,"DSICloseSession\n");
	if (DSICloseSession(Conn)) {
		failed();
		return ExitCode;
	}
	CloseClientSocket(Dsi->socket);

	/* ------------------------ */	
    /* guest login */	
    connect_server(Conn);
    Dsi = &Conn->dsi;
    test3();
	CloseClientSocket(Dsi->socket);
    
	/* ------------------------ 
	 * clear text login
	*/
    connect_server(Conn);
    Dsi = &Conn->dsi;
    if (Version >= 30) {
		ret = FPopenLoginExt(Conn, vers, uam, User, Password);
	}
	else {
		ret = FPopenLogin(Conn, vers, uam, User, Password);
	}
	if (ret) {
		failed();
		return ExitCode;
	}
	Conn->afp_version = Version;
	
   	if (FPLogOut(Conn)) {
   		failed();
   	}

	/* ------------------------ 
	 * too many login
	*/
	test4();

	return ExitCode;
}

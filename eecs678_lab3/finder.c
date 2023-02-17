#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <sys/wait.h>

#define BSIZE 256

#define BASH_EXEC  "/bin/bash"
#define FIND_EXEC  "/bin/find"
#define XARGS_EXEC "/usr/bin/xargs"
#define GREP_EXEC  "/bin/grep"
#define SORT_EXEC  "/bin/sort"
#define HEAD_EXEC  "/usr/bin/head"

int main(int argc, char *argv[])
{
  int status;
  pid_t pid_1, pid_2, pid_3, pid_4;
  
  int p1[2], p2[2], p3[2], p4[2];

  //Checking if input parameters DIR, STR and NUM_FILES are correct
  if (argc != 4) {
    printf("usage: finder DIR STR NUM_FILES\n");
    exit(0);
  }

  //STEP 1
	//Initialize pipes p1, p2, and p3
  pipe(p1);
  pipe(p2);
  pipe(p3);
  pipe(p4);

  pid_1 = fork();
  if (pid_1 == 0) {
    /* First Child */
    char cmdbuf[ BSIZE ];
    bzero( cmdbuf, BSIZE );
    //find
    sprintf( cmdbuf, "%s %s -name \'*\'.[ch]", FIND_EXEC, argv[1] );


    //STEP 2
		//In this first child process, we want to send everything that is printed on the standard output, to the next child process through pipe p1
		//So, redirect standard output of this child process to p1's write end - written data will be automatically available at pipe p1's read end
		//And, close all other pipe ends except the ones used to redirect the above OUTPUT (very important)

    //use dup2 for the STDOUT_FILENO from the write end
    dup2( p1[1], STDOUT_FILENO );

    close( p1[0] );
    //don't close p1[1] since we're using it
    close( p2[0] );
    close( p2[1] );
    close( p3[0] );
    close( p3[1] );
    close( p4[0] );
    close( p4[1] );

    //STEP 3
    //Prepare a command string representing the find command (follow example from the slide)
    //Invoke execl for bash and find (use BASH_EXEC and FIND_EXEC as paths)

    if ( ( execl( BASH_EXEC, FIND_EXEC, "-c", cmdbuf, ( char * ) 0 ) ) < 0 ) {
      fprintf( stderr, "\nError executing find. ERROR#%d\n", errno );
      return EXIT_FAILURE;
    }
    exit(0);
  }

  pid_2 = fork();
  if (pid_2 == 0) {
    /* Second Child */

    char cmdbuf[ BSIZE ];
    bzero( cmdbuf, BSIZE );
    //grep
    sprintf( cmdbuf, "%s %s -c %s", XARGS_EXEC, GREP_EXEC, argv[2] );

    //STEP 4
    //In this second child process, we want to receive everything that is available at pipe p1's read end, and use the received information as standard input for this child process
		//In this second child process, we want to send everything that is printed on the standard output, to the next child process through pipe p2
		//So, redirect standard output of this child process to p2's write end - written data will be automatically available at pipe p2's read end
		//And, close all other pipe ends except the ones used to redirect the above two INPUT/OUTPUT (very important)

    dup2(p1[0], STDIN_FILENO);
    dup2(p2[1], STDOUT_FILENO);

    //since we want the data from p1's write end, don't close it
    close( p1[1] );
    close( p2[0] );
    //also don't close p2[1] since we want to send data there
    close( p3[0] );
    close( p3[1] );
    close( p4[0] );
    close( p4[1] );


    //STEP 5
    //Invoke execl for xargs and grep (use XARGS_EXEC and GREP_EXEC as paths)

    if ( ( execl( XARGS_EXEC, XARGS_EXEC, GREP_EXEC, "-c", cmdbuf, ( char * ) 0 ) ) < 0 ) {
      fprintf( stderr, "\nError execing grep. ERROR#%d\n", errno );
      return EXIT_FAILURE;
    }

    exit(0);
  }

  pid_3 = fork();
  if (pid_3 == 0) {
    /* Third Child */

    // char cmdbuf[ BSIZE ];
    // bzero( cmdbuf, BSIZE );
    // //sort
    // sprintf( cmdbuf, "%s sort -t : +1.0 -2.0 --numeric --reverse", SORT_EXEC ); 

    dup2(p2[0], STDIN_FILENO);
    dup2(p3[1], STDOUT_FILENO);

    //STEP 6
		//In this third child process, we want to receive everything that is available at pipe p2's read end, and use the received information as standard input for this child process
		//In this third child process, we want to send everything that is printed on the standard output, to the next child process through pipe p3
		//So, redirect standard output of this child process to p3's write end - written data will be automatically available at pipe p3's read end
		//And, close all other pipe ends except the ones used to redirect the above two INPUT/OUTPUT (very important)
    close( p1[0] );
    close( p1[1] );
    //close( p2[0] );     //don't use p2[0] as it's the read end
    close( p3[1] );
    close( p3[0] );
    close( p3[1] );     //don't use p3[1] as it's the write end

    //STEP 7
    //Invoke execl for sort (use SORT_EXEC as path)
    if ( ( execl( SORT_EXEC, SORT_EXEC, "-t", ":", "+1.0", "-2.0", "--numeric", "--reverse", (char*) 0)) < 0 ) {
      fprintf( stderr, "\nError execing sort. ERROR#%d\n", errno );
      return EXIT_FAILURE;
    }

    exit(0);
  }

  pid_4 = fork();
  if (pid_4 == 0) {
    /* Fourth Child */
    char cmdbuf[ BSIZE ];
    bzero( cmdbuf, BSIZE );
    //head
    sprintf(cmdbuf, "%s --lines=%s", HEAD_EXEC, argv[3]);

    dup2(p3[0], STDIN_FILENO);

    //STEP 8
		//In this fourth child process, we want to receive everything that is available at pipe p3's read end, and use the received information as standard input for this child process
		//Output of this child process should directly be to the standard output and NOT to any pipe
		//And, close all other pipe ends except the ones used to redirect the above INPUT (very important)
    close( p1[0] );
    close( p1[1] );
    close( p2[0] );     
    close( p3[1] );
    //close( p3[0] );
    close( p3[1] );     
    close( p4[0] );
    close( p4[1] );

    //STEP 8
    //Invoke execl for head (use HEAD_EXEC as path)
    if ( ( execl( HEAD_EXEC, HEAD_EXEC, "-c", cmdbuf, ( char * ) 0 ) ) < 0 ) {
      fprintf( stderr, "\nError executing head. ERROR#%d\n", errno );
      return EXIT_FAILURE;
    }

    exit(0);
  }

  if ((waitpid(pid_1, &status, 0)) == -1) {
    fprintf(stderr, "Process 1 encountered an error. ERROR%d", errno);
    return EXIT_FAILURE;
  }
  if ((waitpid(pid_2, &status, 0)) == -1) {
    fprintf(stderr, "Process 2 encountered an error. ERROR%d", errno);
    return EXIT_FAILURE;
  }
  if ((waitpid(pid_3, &status, 0)) == -1) {
    fprintf(stderr, "Process 3 encountered an error. ERROR%d", errno);
    return EXIT_FAILURE;
  }
  if ((waitpid(pid_4, &status, 0)) == -1) {
    fprintf(stderr, "Process 4 encountered an error. ERROR%d", errno);
    return EXIT_FAILURE;
  }

  return 0;
}

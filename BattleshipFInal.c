#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

#define NO_ITEMS 20
int fd, n, m, buffer = 0;
int user1[10][10] = {0};
int hits_user1[10][10] = {0};
int user2[10][10] = {0};
int hits_user2[10][10] = {0};
int no_hits[2] = {6, 6};
int no_ships = 3;
int cols = 10;
int rows = 10;

pthread_mutex_t mutex;
pthread_cond_t cond_consumer, cond_producer;

//Prints board
void print_board(int board[10][10]) 
{
  //Top Index
  printf("    ");
  for (int i = 1; i <= rows; i++) 
  {
      printf("%3d ", i);
  }
  printf("\n");
  for (int i = 0; i < cols; i++) 
  {
      printf("%3d ", i+1); //Left Index
      for (int j = 0; j < cols; j++) 
      {
          //Character representation
          if (board[i][j] == 0)//Sea
          {
              printf("%3c ", '=');
          }
          else if (board[i][j] == 1)//Ship
          {
              printf("%3c ", 'B');
          }
          else if (board[i][j] == 2)//Missed shot
          {
              printf("%3c ", '/');
          }
          else if (board[i][j] == 3)//Strike a ship/sunken
          {
              printf("%3c ", '*');
          }
          else 
          {
              printf("%3d ", board[i][j]);
          }
      }
      printf("\n");
  }
}

//Placement of 2x2 ships
void placing_ships(int board[10][10], int p)
{
  int x, y, orientation;
  printf("\nCOMMAND CENTER %d\n", p);
  printf("\nGeneral, we need your help!");
  //Placing a ship in the board
  for (int i = 0; i < no_ships; i++) 
  {
    printf("\nPlease enter the coordinates of the ship you want to position #%d: ", i+1);
    scanf("%d %d", &x, &y);

    //Orientation
    printf("\nNow please, enter the desired orientation of the ship. 1 for horizontal orientation and 0 for vertical orientation: ");
    scanf("%d", &orientation);

    //Vertical Check
    if (orientation == 0) 
    {
      if (y < 1 || y > cols) 
      {
        printf("\nI'm afraid I cannot let you position your ship there General. Invalid column number. Try again!\n");
        i--;
        continue;
      }

      if (x < 1 || x > rows-1) 
      {
        printf("\nI'm afraid I cannot let you position your ship there General. Invalid row number. Try again!\n");
        i--;
        continue;
      }

      //Occupied space
      if (board[x-1][y-1] == 1 || board[x][y-1] == 1) 
      {
        printf("\nI'm afraid I cannot let you position your ship there General.The coordinates are already occupied.\n");
        i--;
        continue;
      }

      //Placement of a 2x2 ship vertically
      board[x-1][y-1] = 1;
      board[x][y-1] = 1;
    } 
    //Horizontal Check
    else if (orientation == 1) 
    {
      if (y < 1 || y > cols-1) 
      {
        printf("\nI'm afraid I cannot let you position your ship there General. Invalid column number. Try again!\n");
        i--;
        continue;
      }

      if (x < 1 || x > rows) 
      {
        printf("\nI'm afraid I cannot let you position your ship there General. Invalid row number. Try again!\n");
        i--;
        continue;
      }

      //Occupied space
      if (board[x-1][y-1] == 1 || board[x-1][y] == 1) 
      {
        printf("\nI'm afraid I cannot let you position your ship there General.The coordinates are already occupied.\n");
        i--;
        continue;
      }

      //Placement of a 2x2 ship horizontally
      board[x-1][y-1] = 1;
      board[x-1][y] = 1;
    } 
    else 
    {
      printf("\nI'm afraid I cannot let you position your ship there General. Invalid orientation.\n");
      i--;
      continue;
    }
  }
}

//Entered is expected to continue
void wait_for_enter()
{
  printf("\nTo proceed press ENTER...");
  getchar(); //recieves and consumes enter.
}

//Checks attack of player in turn
void hit_or_miss(int user_a[10][10], int hits_a[10][10], int a, int b)
{
	
  if(no_hits[b-1] == 0 ) //No more ships
  {
  	if (a == 1)
  	{
  		pthread_kill(pthread_self(), SIGUSR1);
	}
    else if (a == 2)
    {
    	pthread_kill(pthread_self(), SIGUSR2);
	}
  }
  
  int x, y;
  printf("\nGENERAL %d", a);
  printf("\n\nTime to strike! Enter the coordinates to strike: ");
  scanf("%d %d", &x, &y);
  getchar();
  if (user_a[x-1][y-1] == 1) //General hit
  {
    user_a[x-1][y-1] = 3;
    hits_a[x-1][y-1] = 3;
    sleep(1);
    no_hits[b-1]--;
    //no_hits2--;
    printf("\nThe attack of GENERAL %d made contact with a ship.\nTime to take the advantage! Strike again!\n", a);
    sleep(2);
    hit_or_miss(user_a, hits_a, a, b);
  }
  else //General miss
  {
    hits_a[x-1][y-1] = 2;
    sleep(1);
    printf("\nThe attack of GENERAL %d failed. Attack failed, you will get em next time.\nNext turn!\n", a);
  }
}

//prints winner
void signal_handler(int signum)
{
  if (signum == SIGUSR1) 
  {
        int victory = 1;
        fd = creat("victory.txt", 777);
        n = write(fd, &victory, sizeof(victory));
        close(fd);
        exit(0);
  }
  else if (signum == SIGUSR2)
  {
        int victory = 2;
        fd = creat("victory.txt", 777);
        n = write(fd, &victory, sizeof(victory));
        close(fd);
        exit(0);
  }
}

//General 1 is producer
void* producer_function(void* arg)
{
  for ( int i=0; i<NO_ITEMS; i++ ) 
  {
    pthread_mutex_lock(&mutex);
    while( buffer != 0 ) //Waits for buffer
    {
      //Release of mutex and waiting of conditions to be met
      pthread_cond_wait( &cond_producer, &mutex );
    }
    buffer = 1;
    
    printf("\n\nRADAR OF GENERAL 2\n");
    print_board(hits_user2);
    printf("\nRADAR OF GENERAL 1\n");
    print_board(user1);
    hit_or_miss(user2, hits_user2, 1, 2);
    wait_for_enter();
    
    
    //checks  buffer
    pthread_cond_signal( &cond_consumer );
    pthread_mutex_unlock( &mutex );
  }

  //Sending of signal to producer onced finished.
  pthread_kill( pthread_self(), SIGUSR1 );
  pthread_exit( NULL );
}

//General 2 is consumer
void* consumer_function(void* arg)
{
  for ( int i=0; i<NO_ITEMS; i++ ) 
  {
    pthread_mutex_lock( &mutex );
    // ---------- Critical region
    while (buffer == 0) //Waits for buffer
    {
      //Release of mutex and waiting of conditions to be met
      pthread_cond_wait( &cond_consumer, &mutex );
    }
    printf("\n\n\nRADAR OF GENERAL 1\n");
    print_board(hits_user1);
    printf("\nRADAR OF GENERAL 2\n");
    print_board(user2);
    hit_or_miss(user1, hits_user1, 2, 1);
    wait_for_enter();
    
  
    buffer = 0;
    
    //checks buffer
    pthread_cond_signal( &cond_producer ); 
    pthread_mutex_unlock( &mutex );
  }

  //Sending of signal to consumer onced finished.
  pthread_kill( pthread_self(), SIGUSR2 );

  pthread_exit(NULL);
}

//Hit or miss confirmation
void alarm_handler(int signum) {}

void menu()
{
	printf("Alejandro Sanchez Gonzalez / ID:167299\n\n");
    printf("WELCOME TO THE BASE GENERAL! WE REQUIRE YOUR ASSISTANCE FOR BATTLESHIP!\n");
    printf("INSTRUCTIONS: \n");
    printf("-THIS IS A 2 PLAYER GAME.\n-EACH GENERAL WILL POSITION 3 SHIPS ON THE BOARD.\n-IT IS A TURNBASED GAME.\n");
    printf("-EACH BOAT OCCUPIES 2 SLOTS, ORIENTATION WILL BE ASKED AFTER SELECTING THE INITIAL COORDINATE.\n");
    printf("-IF ORIENTATION IS VERTICAL, THE BOAT WILL BE EXPANDED DOWNWARDS. IF ORIENTATION IS HORIZONTAL, THE BOAT WILL BE EXPANDED TO THE RIGHT.\n");
    printf("--\n");
    
    printf("B = SHIP\n\n");
    printf("/ = MISSED STRIKE\n\n");
    printf("* = SUNKEN SHIP\n\n");
    printf("= = WATER\n\n\n");
    wait_for_enter();
}

int main(void)
{
    int fd, m;
    int who_won;
    int status;
    pid_t pid_A;
    pthread_t thread_1, thread_2;
    signal(SIGALRM, alarm_handler);
    pid_A = fork();
    
    if (pid_A == 0)
    {
        menu();
        printf("\nRADAR OF GENERAL 1\n");
        print_board(user1);
        placing_ships(user1, 1);
        printf("\nRADAR OF GENERAL 2\n");
        print_board(user2);
        placing_ships(user2, 2);
          
        signal(SIGUSR2, signal_handler);
        signal(SIGUSR1, signal_handler);
          
        pthread_mutex_init( &mutex, 0 );
        pthread_cond_init( &cond_consumer, 0 );
        pthread_cond_init( &cond_producer, 0 );
          
        //Thread creation
        pthread_create( &thread_1, NULL, producer_function, NULL );
        pthread_create( &thread_2, NULL, consumer_function, NULL );
          
        //Wait for threads
        pthread_join( thread_1, NULL);
        pthread_join( thread_2, NULL);
        
        //Destruction of mutex and conditions
        pthread_mutex_destroy( &mutex );
        pthread_cond_destroy( &cond_consumer );
        pthread_cond_destroy( &cond_producer );
        return 0;
    }
    else if (pid_A > 0)
    {
        //Waits for son, then prints winner
        waitpid(pid_A, &status, 0);
        printf("WE FINALLY HAVE A VICTOR:\n");
        fd = open("victory.txt", 0);
        m = read (fd, &who_won, sizeof(who_won));
        alarm(3);
        pause();
        printf("GENERAL %d, YOU ARE VICTORIOUS!!\n", who_won);
        return 0;
    }
}

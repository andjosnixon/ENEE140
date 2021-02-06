	/*
	Project 3 
	
	Andrew Nixon
	
	Fall 2019 
	
	Project description: 
	The purpose of this project is to design and implement a simplified version of Monopoly. 
	The game board is 20x20, which will be studded with properties, which both the computer 
	and the user are allowed to buy. If either player lands on a "chance" location either, then 
	they must follow instructions. Earn as much money as possible, and the game ends when one
	player runs out of money. 

	argv[] position in command line:
	|   0     |   	 	1      |    	  2      |        	 3        |     	4      | 
	  a.out		path_length		path_file		property_file		chance_file
	  
	USER SPECIFIED:  
		a.out: executable
		path_length: length on the path board
	INPUT FILES: 
		path_file: file containing sets of coordinates for the game board
		property_file: file containing sets of coordinates for the properties
		chance_file: coordaintes of where the chance card will be performed. 
	*/
	
	#include <stdio.h>
	#include <time.h>
	#include <stdlib.h>
	
	
	#define N 20
	#define balance_initial 1500
	#define pass_go 200
	#define max_properties 80
	#define max_chances 80
	
	int main(int argc, char *argv[]){
	
		srand(time(NULL));
		
	//checking if the number of command line arguments are ok
		
		if(argc != 5){ 
			printf("Incorrect number of arguments provided.\n");
			printf("Usage: a.out, path_length, path_file, ");
			printf("property_file, chance_file");
			exit(0);
		}
	//safety checks
		FILE *path_file;
			path_file = fopen(argv[2], "r");
			//formatted as (y1 x1) -> (y2 x2)
			//first and last coordinates are (0,0)
		FILE *property_file; 
			property_file = fopen(argv[3], "r");
			//formatted as (y x) p1 p2 p3 | r1 r2 r3
			//px are purchase prices 
			//rx are rental prices 
		FILE *chance_file; 
			chance_file = fopen(argv[4], "r");
			// formatted as (y x)
			// first line is always JAIL in case player picks a card that says go to jail. 
		if(path_file == NULL){
			printf("%s could not be opened properly.\n", argv[2]); 
			exit(0);
		}
		if(property_file == NULL){
			printf("%s could not be opened properly.\n", argv[3]);
			exit(0);
		}
		if(chance_file == NULL){
			printf("%s could not be opened properly.\n", argv[4]);
			exit(0);
		}	
	// Data structures for the project
		int path_length = atoi(argv[1]);
		int step[path_length][2];
		char board[N+1][N+1];
		int from_x[path_length], from_y[path_length], to_x[path_length], to_y[path_length];
		int p1_balance = balance_initial, p2_balance = balance_initial; 
		int turn_number=1, whose_turn = 1, die1, die2, forward;
		char player_one = 'A', player_two = 'B';
		int i, j, k, x, y, cnt, num_properties, done=0;
		int prop_x[max_properties], prop_y[max_properties]; 	
		int properties[max_properties][3], rentals[max_properties][3];
		int chance_x[max_chances], chance_y[max_chances];
		int player_one_coor[] = {0, 0}, player_two_coor[] = {0, 0}; 
		char roll, nobody[] = "nobody";
		int p1_answer, p2_answer;
		int player_one_line = 0, player_two_line = 0, player_one_laps = 0, player_two_laps=0;
		int chance_draw, temp, player_one_in_jail = 0, player_two_in_jail=0;
		int p1_extra_turn=0, p2_extra_turn=0, p1_lose_turn = 0, p2_lose_turn = 0;
		int upgrade_stage; 
		//player_xxx_props[max number of total properties[0 = property value, 1 = property stage]
		int player_one_props[max_properties][2], player_two_props[max_properties][2], player_one_rent[max_properties][2], player_two_rent[max_properties][2];
			for(i=0;i<max_properties;i++){
				for(j=0;j<2;j++){
					player_one_props[i][j]=0;
					player_two_props[i][j]=0;
					player_one_rent[i][j]=0;
					player_two_rent[i][j]=0;
				}
			}
		int prop_purchase_record[max_properties];
			for(i=0;i<max_properties;i++){
				prop_purchase_record[i] = 0;
			}

	//scanning in the board data
		for(i=0; !feof(path_file); i++){
			fscanf(path_file, "(%d %d) -> (%d %d)\n", &(from_y[i]), &(from_x[i]), &(to_y[i]), &(to_x[i]));
		}
	//scanning in the property data
		for(i=0; !feof(property_file); i++){
			fscanf(property_file, "(%d %d) %d %d %d | %d %d %d\n", &prop_y[i], &prop_x[i], &properties[i][0], &properties[i][1], &properties[i][2], &rentals[i][0], &rentals[i][1], &rentals[i][2]);
		}
		num_properties=i;
	//scanning in the chance data
		for(i=0; !feof(chance_file); i++){
			fscanf(chance_file, "(%d %d)\n", &(chance_y[i]), &(chance_x[i]));
		}
	//initiating the step file:
		for(i=0; i<path_length; i++){
			step[i][0] = from_y[i];
			step[i][1] = from_x[i];
		}
			
	//adding the boarder 
		//zeroing the array first
		for(i=0; i<N+1; i++){
			for(j=0; j<N+1; j++){
				board[i][j] = 32;
			}
		}
		//adding the numbers
		for(i=1; i<N+1; i++){ 
			if(i<11){
				board[i][0] = i+47;
				board[0][i] = i+47;
			}
			else{
				board[i][0] = i+37;
				board[0][i] = i+37;
			}
		}
	//assigning movement characters 	
		for(i=0; i<path_length; i++){
			if(from_x[i] == to_x[i]){
				if(from_y[i] > to_y[i]){
					board[to_y[i]+1][from_x[i]+1] = '^';
				}
				else{
					board[to_y[i]+1][from_x[i]+1] = 'v';
				}
			}
			else{ 
				if(from_x[i] > to_x[i]){
					board[from_y[i]+1][to_x[i]+1] = '<';
				}
				else{
					board[from_y[i]+1][to_x[i]+1] = '>';
				}
			}
		}
		
	//gameplay
		printf("\n\n\n\nWelcome to the game of the monopoly. Jail is located at position (%d, %d)\n\n\n\n", chance_y[0], chance_x[0]);
		
		while(turn_number<100){	
		
			printf("~~~~~~~~BEGINNING OF TURN NUMBER %d~~~~~~~~~~~\n", turn_number);
			printf("-----Player Balances------ \n");
			printf("Player 1: %d, Player 2: %d\n", p1_balance, p2_balance);
			printf("-----Player Coordinates---\n");
			printf("Player 1: (%d,%d), Player 2: (%d,%d)\n", player_one_coor[0], player_one_coor[1], player_two_coor[0], player_two_coor[1]);
			printf("-----Player 1 Properties---- \n");
			printf("Property \t Upgrade Stage \t Value \t Rent");
			for(i=0; i<num_properties; i++){
				if(player_one_props[i][0]>0){
					printf("%d \t %d \t %d \t %d\n", i, player_one_props[i][1], player_one_props[i][0], player_one_rent[i][0]);
				}
			}
			printf("-----Player 2 Properties---- \n");
			printf("Property \t Upgrade Stage \t Value \t Rent");
			for(i=0; i<num_properties; i++){
				if(player_two_props[i][0]>0){
					printf("%d \t %d \t %d \t %d\n", i, player_two_props[i][1], player_two_props[i][0], player_two_rent[i][0]);
				}
			}
			
			printf("It is player %d's turn\n\n", whose_turn);
			
		// checking to see if any players are in jail
			if(player_one_in_jail){
				printf("Player 1 is in jail! You must roll doubles to get out.\n");
			}
			if(player_two_in_jail){
				printf("Player 2 is in jail! They must roll doubles to get out.\n");
			}
			if(!player_one_in_jail && !player_two_in_jail){
				printf("nobody is in jail\n");
			}
		
		//print the board at the start of the turn
			for(i=0; i<N+1; i++){
				for(j=0; j<N+1; j++){
					if(player_one_coor[0]+1==i && player_one_coor[1]+1==j && player_one_coor[0]==player_two_coor[0] && player_one_coor[1] == player_two_coor[1]){
						printf("%c%-2c", board[i][j], 'E');
					}
					else if(player_one_coor[0]+1==i && player_one_coor[1]+1==j){
						printf("%c%-2c", board[i][j], 'A');
					}
					else if(player_two_coor[0]+1==i && player_two_coor[1]+1==j){
						printf("%c%-2c", board[i][j], 'B');
					}
					else{
						printf("%-3c", board[i][j]);
					}
				}
				printf("\n");
			}
			//rolling the dice 
			if(whose_turn == 1){
				printf("press enter to let player 1 roll the dice.");
				if(scanf("%c", &roll)==1){
					die1 = rand()%6+1;
					die2 = rand()%6+1;
				}
			}
			else{
				printf("press enter to let player 2 roll the dice.");
				if(scanf("%c", &roll)==1){
					die1 = rand()%6+1;
					die2 = rand()%6+1;
				}
			}
			printf("player %d rolled a %d and a %d\n", whose_turn, die1, die2);
			
			if(whose_turn == 1){
				if(player_one_in_jail){
					if(die1==die2){
					printf("player 1 rolled doubles! you're allowed to leave.\n\n");
					player_one_in_jail = 0;
					}
					else{
					printf("sorry, you didn't roll doubles. keep trying.\n");
					}
				}
			}
			else{
				if(player_two_in_jail){
					if(die1==die2){ 
					printf("player 2 rolled doubles! they're allowed to leave.\n\n");
					player_two_in_jail = 0;
					}
					else{
					printf("sorry, you didn't roll doubles. keep trying.\n");
					}
				}
			}
			
			forward = die1 + die2; 
			
		//moving if it's player 1's turn and they're not in jail	
			if(whose_turn==1){
				if(!player_one_in_jail){
					printf("move forward %d steps\n", forward);
					for(i=0; i<forward+1; i++){
						if(player_one_line+i>72){
							player_one_line = player_one_line - 72;
							player_one_laps++;
							p1_balance = p1_balance + pass_go;
						}
						player_one_coor[0] = step[player_one_line+i][0];
						player_one_coor[1] = step[player_one_line+i][1];
					}
					player_one_line = player_one_line + forward;
				}
			}
		//moving if it's player 2's turn and they're not in jail	
			else{
				if(!player_two_in_jail){
					printf("move forward %d steps\n", forward);
					for(i=0; i<forward+1; i++){
						if(player_two_line+i>72){
							player_two_line = player_two_line - 72;
							player_two_laps++;
							p2_balance = p2_balance + pass_go;
						}
						player_two_coor[0] = step[player_two_line+i][0];
						player_two_coor[1] = step[player_two_line+i][1];
					}
					player_two_line = player_two_line + forward;
				}
			}
			
		//chance and properties for player 1	
			if(whose_turn==1){
				if(!player_one_in_jail){
					
					
					//testing for chance cards
					for(i=0; i<7; i++){
						if(player_one_coor[0] == chance_y[i] && player_one_coor[1] == chance_x[i]){
							printf("player 1, you have landed on a chance card.\n");
							chance_draw = rand()%7 + 1;
							switch(chance_draw){
								case 1: 
									printf("you're going to jail.\n");
									player_one_coor[0] = 1;
									player_one_coor[1] = 3;
									player_one_in_jail=1;
									player_one_line=5;
									break;
								case 2:
									printf("chaos ensues. trade balances.\n");
									temp = p1_balance;
									p1_balance = p2_balance;
									p2_balance = temp;
									break;
								case 3:
									printf("you get an extra turn.\n");
									p1_extra_turn=1;
									break;
								
								case 4: 
									printf("you lose a turn.\n");
									p1_lose_turn=2;
									break;
								case 5: 
									printf("the government takes half your money\n");
									p1_balance = p1_balance/2;
									break;
								case 6: 
									printf("You bought stuff on amazon that you didn't need to. Lose $200\n");
									p1_balance= p1_balance - 200;
									break;
								default:
									printf("advance to go and collect $200.\n");
									player_one_coor[0] = 0;
									player_one_coor[1] = 0;
									p1_balance = p1_balance+pass_go;
									player_one_laps++;
									break;
							}
						}
					}
					
				//player 1 properties 
					for(i=0; i<num_properties; i++){
						if(player_one_coor[0] == prop_y[i] && player_one_coor[1] == prop_x[i]){
							printf("player 1, you have landed on property #%d. \n", i+1);
							printf("Space \t Owner \t Upgrade Stage \t Upgrade Cost \t Rental Cost\n");
							printf("printf(%d, %d) \t ", prop_y[i], prop_x[i]);
							
							if(prop_purchase_record[i] == 0){
								printf("%s \t 1 \t", nobody);
								printf("$%d \t $%d ", properties[i][0], rentals[i][0]);
							}
							else if(prop_purchase_record[i] == 1){
								upgrade_stage = player_one_props[i][1];
								printf("P1 \t %d \t ", upgrade_stage); 
								if(upgrade_stage==3){
									printf("N/A /t");
								}
								else{
									printf("%d \t", properties[i][upgrade_stage-1]);
								}
								printf("%d /t", rentals[i][upgrade_stage-1]);
							}
							else{
								printf("P2 \t %d \t ", upgrade_stage); 
								if(upgrade_stage==3){
									printf("N/A /t");
								}
								else{
									printf("%d \t", properties[i][upgrade_stage-1]);
								}
								printf("%d /t", rentals[i][upgrade_stage-1]);
							}
						}
						printf("\n");
						if(prop_purchase_record[i] == 0){
							if(p1_balance > properties[i][0]){ 
								printf("would you like to upgrade? Enter 1 for yes or 0 for no.");
								scanf("%d", &p1_answer);
								if(p1_answer==0){
									printf("you have declinded to upgrade");
								}
								else if(p1_answer==0){
									printf("you have decided to upgrade.");
									p1_balance = p1_balance - properties[i][0];
									prop_purchase_record[i] = 1;
									player_one_props[i][0]=properties[i][0];
									player_one_props[i][1] = 1;
									player_one_rent[i][0] = rentals[i][0];
									player_one_rent[i][1] = 1;
								}
								else{
									printf("you didn't enter a 1 or a 0 so you decided not to upgrade.\n");
								}
							}
						}
						else if(prop_purchase_record[i] == 1){
							printf("you already own this property. ");
							if(player_one_props[i][1] == 3){
								printf("this property is already at tier 3, it cannot be upgraded.\n");
							}
							else if(player_one_props[i][1] == 2){
								if(p1_balance > properties[i][2]){
									printf("you're at tier 2 and you can afford tier 3. would you like to upgrade? ");
									printf("enter 1 for yes or 0 for no.");
									scanf("%d", &p1_answer);
									if(p1_answer==0){
									printf("you have declinded to upgrade");
									}
									else if(p1_answer==0){
										printf("you have decided to upgrade.");
										p1_balance = p1_balance - properties[i][2];
										player_one_props[i][0]=properties[i][2];
										player_one_props[i][1]= 3;
										player_one_rent[i][0] = rentals[i][2];
										player_one_rent[i][1]= 3;
									}
									else{
										printf("you didn't enter a 1 or a 0 so you decided not to upgrade.\n");
									}
								}
							}
							else{
								if(p1_balance > properties[i][1]){
									printf("you're at tier 1 and you can afford tier 2. would you like to upgrade? ");
									printf("enter 1 for yes or 0 for no.");
									scanf("%d", &p1_answer);
									if(p1_answer==0){
									printf("you have declinded to upgrade");
									}
									else if(p1_answer==0){
										printf("you have decided to upgrade.");
										p1_balance = p1_balance - properties[i][1];
										player_one_props[i][0] = properties[i][1];
										player_one_props[i][1] = 2;
										player_one_rent[i][0] = rentals[i][1];
										player_one_rent[i][1] = 2;
									}
									else{
										printf("you didn't enter a 1 or a 0 so you decided not to upgrade.\n");
									}
								}
							}
						}
						else{
							printf("you must pay rent in the amount of %d\n", player_two_rent[i][0]);
							p1_balance = p1_balance - player_two_rent[i][0];
						}
					}
				}
			}
		//chance and properties for player 2
			else{
				if(!player_two_in_jail){
					//testing for chance cards
					for(i=0; i<7; i++){
						if(player_two_coor[0] == chance_y[i] && player_two_coor[1] == chance_x[i]){
						printf("player 2 has landed on a chance card.\n");
						chance_draw = rand()%7 + 1;
							switch(chance_draw){
								case 1: 
									printf("you're going to jail.\n");
									player_two_coor[0] = 1;
									player_two_coor[1] = 3;
									player_two_in_jail = 1;
									player_two_line=5;
									break;
								
								case 2:
									printf("chaos ensues. trade balances.\n");
									temp = p1_balance;
									p1_balance = p2_balance;
									p2_balance = temp;
									break;
								
								case 3:
									printf("you get an extra turn.\n");
									p2_extra_turn=1;
									break;
								
								case 4: 
									printf("you lose a turn.\n");
									p2_lose_turn = 2;
									break;
								
								case 5: 
									printf("the government takes half your money\n");
									p2_balance = p2_balance/2;
									break;
								
								case 6: 
									printf("Player 2 shopped on amazon and spent too much. Lose $200.\n");
									p2_balance = p2_balance - 200;
									break;
								
								default:
									printf("advance to go and collect $200.\n");
									player_two_coor[0] = 0;
									player_two_coor[1] = 0;
									p2_balance = p2_balance+pass_go;
									player_two_laps++;
									break;
							}
						}
					}
					
					//player 2 properties 
					for(i=0; i<num_properties; i++){
						if(player_two_coor[0] == prop_y[i] && player_two_coor[1] == prop_x[i]){
							printf("player 2 has landed on property #%d. \n", i+1);
							printf("Space \t Owner \t Upgrade Stage \t Upgrade Cost \t Rental Cost\n");
							printf("printf(%d, %d) \t ", prop_y[i], prop_x[i]);
							
							if(prop_purchase_record[i] == 0){
								printf("%s \t 1 \t", nobody);
								printf("$%d \t $%d ", properties[i][0], rentals[i][0]);
							}
							else if(prop_purchase_record[i] == 1){
								upgrade_stage = player_one_props[i][1];
								printf("P1 \t %d \t ", upgrade_stage); 
								if(upgrade_stage==3){
									printf("N/A /t");
								}
								else{
									printf("%d \t", properties[i][upgrade_stage-1]);
								}
								printf("%d /t", rentals[i][upgrade_stage-1]);
							}
							else{
								printf("P2 \t %d \t ", upgrade_stage); 
								if(upgrade_stage==3){
									printf("N/A /t");
								}
								else{
									printf("%d \t", properties[i][upgrade_stage-1]);
								}
								printf("%d /t", rentals[i][upgrade_stage-1]);
							}
						}
						printf("\n");
						if(prop_purchase_record[i] == 0){
							if(p2_balance > properties[i][0]){ 
								printf("this property is unclaimed and player 2 must upgrade.\n");
								p2_balance = p2_balance - properties[i][0];
								player_two_props[i][0] = properties[i][0];
								player_two_props[i][1] = 1;
								player_two_rent[i][0] = rentals[i][0];
								player_two_rent[i][1] = 1;
							}
						}
						else if(prop_purchase_record[i] == 1){
							printf("Player 1 owns this property and player 2 must pay rent.\n");
							p2_balance = p2_balance - player_one_rent[i][0];
						}
						else{
							printf("player 2 already owns this property. ");
							if(player_two_props[i][1] == 3){
								("this property is already maxed out, player 2 cannot upgrade.\n");
							}
							else if(player_two_props[i][1] == 2){
								if(p2_balance > properties[i][2]){
									printf("player 2 is at tier 2 and must upgrade to tier 3.\n");
									p2_balance = p2_balance - properties[i][2];
									player_two_props[i][0] = properties[i][2];
									player_two_props[i][1] = 3;
									player_two_rent[i][2] = rentals[i][2];
									player_two_rent[i][1] = 3;
								}
							}
							else if(player_two_props[i][1] == 1){
								if(p2_balance > properties[i][1]){
									printf("player 2 is at tier 1 and must upgrade to tier 2.\n");
									p2_balance = p2_balance - properties[i][1];
									player_two_props[i][0] = properties[i][1];
									player_two_props[i][1] = 2;
									player_two_rent[i][0] = rentals[i][1];
									player_two_rent[i][1] = 2;
								}
							}
						}
					}
				}
			}
			
			if(p1_extra_turn!=0){
				whose_turn = 1;
				p1_extra_turn--;
			}
			else if(p2_extra_turn!=0){
				whose_turn = 2;
				p2_extra_turn--;
			}
			else if(p1_lose_turn!=0){
				whose_turn = 2;
				p1_lose_turn--;
			}
			else if(p2_lose_turn!=0){
				whose_turn = 1;
				p2_lose_turn--;
			}
			else if(whose_turn==1){
				whose_turn=2;
			}
			else{
				whose_turn=1;
			}
			
			turn_number++;
			
			printf("player one line: %d and player two line: %d\n", player_one_line, player_two_line);
			printf("player one laps: %d and player two laps: %d\n", player_one_laps, player_two_laps);
			printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n");
			
			
		}
		
		return 0;
	}

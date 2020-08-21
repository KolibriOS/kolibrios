#include <stdio.h>
#include <iomanip>
#include <iostream>
#include <string.h>
#include <cstdlib>

struct Card_Node{
	char color;
	char suit;
	int value; // Value = 1 for A, 11 for J, 12 for Q, 13 for K
	bool isup; // true for UP, false for DOWN
	Card_Node *next;
};

struct List_Node{
	int listno;//board and foundation lists have indexes
	List_Node *next;
	Card_Node *cards;
};

struct List{
	Card_Node *top_head;//Top list head
	List_Node *foundation_head;//points first foundation list
	List_Node *board_head;//points first board list
	FILE *fptr;
	
	void create();// Includes create toplist, boardlists and foundlists
	void create_toplist();
	void create_boardlists();
	void create_foundlists();
	void printkart(Card_Node *kart);// prints datas of one kart node
	void printlists();// Prints all lists
	void printmenu();//Prints choice menu
	bool addcardtolist(List_Node *selectedlist, Card_Node *transferedcard, int whichlisttype);//Adds transferedcard to selectedlist, whichlisttype is entered by program
	void goster_TopToFoundation();//Gets input and calls related function
	void TopToFoundation(char s_color, char s_suit, int s_value);// Moves a card top list to foundation list
	void goster_TopToBoard();//Gets input and calls related function
	void TopToBoard(int listindex, char s_color, char s_suit, int s_value);//Moves a card from top list to board list
	void goster_BoardToBoard();//Gets input and calls related function
	void BoardToBoard(int fromwhere, int towhere, char s_color, char s_suit, int s_value);//Moves a card(s) from a fromwhere.list to towhere.list
	void goster_BoardToFoundation();//Gets input and calls related function
	void BoardToFoundation(int fromwhere);//Moves a last card in fromwhere.list of boardlist  to foundation list
	void islem_yap();//Wants user to select choice from menu, and calls related functions
	bool istopempty, isboardempty;// For controlling end of game
	void clear_screen();//Clears screen both windows and linux system
	void close();//Deletes all linked lists and linkedlists's all nodes, closes read txt file
};

List islem;
using namespace std;

int main()
{
	char secim;
	islem.create();
	if(!islem.fptr)//If cannot find solitaire.txt file, close the program
		return 0;

	islem.istopempty = false;//If the top list is empty, this value will be true
	islem.isboardempty = false;//If the board list is empty, this value will be true

	while(1){
		islem.printlists();
		islem.printmenu();
		cin >> secim;
		cin.ignore(10000, '\n');

		if(secim == '1'){
			islem.goster_TopToFoundation();
			cin.ignore(10000, '\n');
			getchar();
		}
		else if(secim == '2'){
			islem.goster_TopToBoard();
			cin.ignore(10000, '\n');
			getchar();
		}
		else if(secim == '3'){
			islem.goster_BoardToBoard();
			cin.ignore(10000, '\n');
			getchar();
		}
		else if(secim == '4'){
			islem.goster_BoardToFoundation();
			cin.ignore(10000, '\n');
		}
		else if(secim == '9'){
			islem.close();
			printf("\n\nThe game is closed and all cards are deleted from memory.Press enter to exit");
			cin.ignore(10000, '\n');
			getchar();
			break;
		}

		else
			cout << "Invalid choice" << endl;

		if(!islem.top_head)// checking top list empty or not
			islem.istopempty = true;
		
		List_Node *traverse;
		traverse = islem.board_head;
		int counter = 0;
		while(traverse){//checking board lists is empty or not
			if(!traverse->cards)
				counter++;
			traverse = traverse->next;
			if(counter == 7)// counter is 7 if all of 7 boardlists are empty
				islem.isboardempty = true;
		}

		/*When game is completed prints message to user*/
		if(islem.isboardempty && islem.istopempty){//Stop while loop when board and top lists are empty
			printf("\n\n\n\t\t\tYOU WIN!!!! THE GAME IS FINISHED! CONGRATULATIONS!\n");// game over message
			printf("\t\t\tYOU WIN!!!! THE GAME IS FINISHED! CONGRATULATIONS!\n");
			printf("\t\t\tYOU WIN!!!! THE GAME IS FINISHED! CONGRATULATIONS!\n");
			printf("\nAll cards are deleted from memory.Press enter to exit");
			cin.ignore(1000, '\n');
			islem.close();
			getchar();
			break;
		}
	}
	
	return 0;
}

void List::create(){
	fptr = fopen("solitaire.txt", "r+");
	if(!fptr){
		cout << "The 'solitaire.txt' file cannot be found. Program will be closed."<< endl;
		getchar();
		return;
	}
	fseek(fptr, 0, SEEK_SET);
	create_toplist();
	create_boardlists();
	create_foundlists();
}//end create function

void List::create_toplist(){
	Card_Node *newnode, *final;
	char tempcolor, tempsuit, tempvalue[4], tempisup[4], garbage[10];//temporary values of color,suit,isup. value will be changed to integer and isup(Up or Down) will be changed to boolean variables
	top_head = NULL;

	while(1){//This while creates top_list with linked list
		newnode = new Card_Node;
		newnode->next = NULL;
		fscanf(fptr, "%c", &tempcolor);
		if(tempcolor == '*'){//first star has read
				fscanf(fptr, "%6c", garbage);//passes other five stars and goes new line
				break;
		}
		else
			newnode->color = tempcolor;

		fscanf(fptr, " %c %s %s ", &tempsuit, tempvalue, tempisup);
		newnode->suit = tempsuit;
		
		/*Changing type of value char array to integer*/
		if(tempvalue[0] == 'A')	newnode->value = 1;
		else if(tempvalue[0] == 'J')newnode->value = 11;
		else if(tempvalue[0] == 'Q')newnode->value = 12;
		else if(tempvalue[0] == 'K')newnode->value = 13;
		else 
			sscanf(tempvalue, "%d", &newnode->value);
		
		/*Changing type of isup char array to boolean*/
		if(strcmp(tempisup, "Up") == 0)
			newnode->isup = true;
		if(strcmp(tempisup, "Down") == 0)
			newnode->isup = false;
		
		if(top_head == NULL){//add first node to empty top list
			top_head = newnode;
			final = top_head;
		}
		else{// add node to not empty list
			final->next = newnode;
			final = final->next;
		}
	}//end while
}

void List::create_boardlists(){
	Card_Node *newnode, *final;// final points to last node of card list
	List_Node *newboardlist, *boardlist_final;// boardlist_final points to last node of board list
	char tempcolor, tempsuit, tempvalue[4], tempisup[8], garbage[10];

	int index = 1;// This index represents nth board list
	
	newboardlist = new List_Node;// creating first boardlist node
	board_head = newboardlist;
	boardlist_final = newboardlist;
	newboardlist->listno = index++;
	newboardlist->next = NULL;
	newboardlist->cards = NULL;

	while(!feof(fptr)){//This while creates board multi linked lists
		newnode = new Card_Node;
		newnode->next = NULL;
		fscanf(fptr, "%c", &tempcolor);
		if(tempcolor == '*'){//first star has read
				fscanf(fptr, "%6c", garbage);//passes other five stars and goes new line
				newboardlist = new List_Node;
				newboardlist->listno = index++;
				newboardlist->next = NULL;
				newboardlist->cards = NULL;
				boardlist_final->next = newboardlist;
				boardlist_final = boardlist_final->next;
				continue;
		}
		else
			newnode->color = tempcolor;

		fscanf(fptr, " %c %s %s ", &tempsuit, tempvalue, tempisup);
		newnode->suit = tempsuit;
		if(tempvalue[0] == 'A')	newnode->value = 1;
		else if(tempvalue[0] == 'J')newnode->value = 11;
		else if(tempvalue[0] == 'Q')newnode->value = 12;
		else if(tempvalue[0] == 'K')newnode->value = 13;
		else 
			sscanf(tempvalue, "%d", &newnode->value);

		if(strcmp(tempisup, "Up") == 0)
			newnode->isup = true;
		if(strcmp(tempisup, "Down") == 0)
			newnode->isup = false;

		if(boardlist_final->cards == NULL){//add first node to empty board list
			boardlist_final->cards = newnode;
			final = boardlist_final->cards;
		}
		else{// add to not empty board list
			final->next = newnode;
			final = final->next;
		}
	}//end while
}

void List::create_foundlists(){
	foundation_head = NULL;
	List_Node *newfoundlist, *foundlist_final;
	int index = 1;// Spades list index = 1, Hearts index = 2, Diamonds = 3, Clubs = 4

	/*Initializing Spades list*/
	newfoundlist = new List_Node;
	newfoundlist->cards = NULL;
	newfoundlist->next = NULL;
	newfoundlist->listno = index++;
	foundation_head = newfoundlist;
	foundlist_final = newfoundlist;

	/*Initializing other three lists*/
	for(int i = 0 ; i <3; i++)	{
		newfoundlist = new List_Node;
		newfoundlist->cards = NULL;
		newfoundlist->next = NULL;
		newfoundlist->listno = index++;
		foundlist_final->next = newfoundlist;
		foundlist_final = foundlist_final->next;
	}//end for
}// end function

void List::printkart(Card_Node *kart){//prints datas of kart node
	if(!kart->isup ){//Hide card if it is down
		printf("X");
		return;
	}
	char a;
	if(kart->value == 1) a='A';
	else if(kart->value == 11) a='J';
	else if(kart->value == 12) a='Q';
	else if(kart->value == 13) a='K';
	else a = '\0';

	if(!a)printf("%c,%c,%d", kart->color, kart->suit, kart->value);
	else printf("%c,%c,%c", kart->color, kart->suit, a);
}

void List::printlists(){
	clear_screen();
	Card_Node *ct[7];// Board List Card Node Traverser; ct[0] for 1.list, ct[1] for 2.list ....
	Card_Node *foundct[4];//Found List Card Node Traverser, foundct[0] = Spades, foundct[1] = Hearts, foundct[2] = Diamonds, foundct[3] = Clubs 
	Card_Node *topct;// TopList Card Traverser
	List_Node *listtraverse;// List Node Traverser


	cout << "Top List:" << endl;
	topct = top_head;
	while(topct){
		printkart(topct);
		printf("|");
		topct = topct->next;
	}

	cout << endl << "\nBoard Lists:" << endl;
	for(int i=1; i <8; i++)
		cout << i << ".List\t";
	cout <<endl;

	listtraverse = board_head;
	for(int i = 0; i < 7; i++){// Initalizing ct[] pointers
		ct[i] = listtraverse->cards;
		listtraverse = listtraverse->next;
	}

	/*Printing Board List's Cards*/
	for(int i = 0; i < 19; i++){//this for loop traverses lists and goes 19 times down, a list can include cards up to 19 (6 down cards + 13 up cards) 
		for(int j = 0; j < 7; j++)
		{
			if(ct[j]){// if ct[j] is not null, print it and go to next node
				printkart(ct[j]);
				printf("\t");
				ct[j] = ct[j]->next;
			}
			else// if ct[j] is null, print a tab
				printf("\t");
		}
		if(ct[0] || ct[1] || ct[2] || ct[3] || ct[4] || ct[5] || ct[6])// After printing a line; 
			printf("\n");//if at least one card is not null: go new line
		else
			break;// if all cards in line are null: break outer for loop
	}//end outer for

	cout << endl << "Foundation Lists:" << endl;
	cout << "Spades\tHearts\tDiamnds\tClubs" << endl;

	listtraverse = foundation_head;
	for(int i = 0; i < 4; i++){// Initalizing foundct[] pointers
		foundct[i] = listtraverse->cards;
		listtraverse = listtraverse->next;
	}

	for(int i = 0; i < 13; i++){//this for loop traverses foundation lists (max 13 cards can be in a foundation list)
		for(int j = 0; j < 4; j++)
		{
			if(foundct[j]){// if ct[j] is not null, print it and go to next node
				printkart(foundct[j]);
				printf("\t");
				foundct[j] = foundct[j]->next;
			}
			else// if foundct[j] is null, print a tab
				printf("\t");
		}
		if(foundct[0] || foundct[1] || foundct[2] || foundct[3])// After printing a line; 
			printf("\n");//if at least one card is not null: go new line
		else
			break;// if all cards in line are null: break outer for loop
	}//end outer for
	printf("\n\n");
}//end function

void List::printmenu(){
	cout << "Choose an operation:" << endl;
	cout << "\t1. Select from Top List to Foundation Lists" << endl;
	cout << "\t2. Select from Top List to Board Lists" << endl;
	cout << "\t3. Move on the Board Lists" << endl;
	cout << "\t4. Move from Board List to Foundation List" << endl;
	cout << "Please enter your choice (1, 2, 3, or 4), (enter 9 to exit):" ;
}

bool List::addcardtolist(List_Node *selectedlist, Card_Node *transferedcard, int whichlisttype){//whichlisttype is not related with user, it is automatically entered by progra
	Card_Node *cardtraverser;
	cardtraverser= selectedlist->cards;
	if(whichlisttype == 1){//whichlisttype is 1 for top to board list
		if(cardtraverser == NULL && transferedcard->value == 13){// if list is empty card's value must be K=13
			selectedlist->cards = transferedcard;
			transferedcard->next = NULL;
			return true;
		}
		if(!cardtraverser)// cardtraverse cannot be null here, can be null only for value K=13
			return false;
		while(cardtraverser->next)
			cardtraverser = cardtraverser->next;

		if(!cardtraverser->isup){// if the card is down, color and value between two cards are not important
			cardtraverser->next = transferedcard;
			transferedcard->next = NULL;
			return true;
		}
		if(cardtraverser->isup)// if the card is up, color and value between two cards are important
			if(!(transferedcard->color == cardtraverser->color))// if colors between two adjacent cards are different
				if(cardtraverser->value - transferedcard->value == 1 ){//list's last value - transfered card's value - must be 1
					cardtraverser->next = transferedcard;
					transferedcard->next = NULL;
					return true;
				}
	}

	if(whichlisttype == 2){// whichlisttype is 2 for moving top to foundation list
		if(cardtraverser == NULL && transferedcard->value == 1){// if list is empty card's value must be A=1
			selectedlist->cards = transferedcard;
			transferedcard->next = NULL;
			return true;
		}
		if(!cardtraverser)// cardtraverse cannot be null here, can be null only for value A=1
			return false;

		while(cardtraverser->next)
			cardtraverser = cardtraverser->next;
		if(transferedcard->value - cardtraverser->value == 1){//if list is not empty, list's last value - card's value must be 1
			cardtraverser->next = transferedcard;
			transferedcard->next = NULL;
			return true;
		}
	}

	if(whichlisttype == 3){//whichlisttype is 3 for board to board list
		if(cardtraverser == NULL && transferedcard->value == 13){// if list is empty card's value must be K=13
			selectedlist->cards = transferedcard;
			return true;
		}
		if(!cardtraverser)// cardtraverse cannot be null here, can be null only for value K=13
			return false;
		while(cardtraverser->next)
			cardtraverser = cardtraverser->next;

		if(!cardtraverser->isup){// if the card is down, color and value between two cards are not important
			cardtraverser->next = transferedcard;
			return true;
		}
		if(cardtraverser->isup)// if the card is up, color and value between two cards are important
			if(!(transferedcard->color == cardtraverser->color))// if colors between two adjacent cards are different
				if(cardtraverser->value - transferedcard->value == 1 ){//list's last value - transfered card's value - must be 1
					cardtraverser->next = transferedcard;
					return true;
				}
	}
	if(whichlisttype == 4){// whichlisttype is 4 for moving board to foundation list
		if(cardtraverser == NULL && transferedcard->value == 1){// if list is empty card's value must be A=1
			selectedlist->cards = transferedcard;
			transferedcard->next = NULL;
			return true;
		}
		if(!cardtraverser)// cardtraverse cannot be null here, can be null only for value A=1
			return false;

		while(cardtraverser->next)
			cardtraverser = cardtraverser->next;
		if(transferedcard->value - cardtraverser->value == 1){//if list is not empty, list's last value - card's value must be 1
			cardtraverser->next = transferedcard;
			transferedcard->next = NULL;
			return true;
		}
	}

	return false;
}

void List::goster_TopToFoundation(){// wants input from use
	char Symbol_of_colors, Symbol_of_suits, tempvalue[4];
	int Symbol_of_numbers;
	cout << endl << "Select a card from Top List:";
	scanf("%c %c %s", &Symbol_of_colors, &Symbol_of_suits, tempvalue);

	if(tempvalue[0] == 'A')	Symbol_of_numbers = 1;
	else if(tempvalue[0] == 'J')Symbol_of_numbers = 11;
	else if(tempvalue[0] == 'Q')Symbol_of_numbers = 12;
	else if(tempvalue[0] == 'K')Symbol_of_numbers = 13;
	else 
		sscanf(tempvalue, "%d", &Symbol_of_numbers);

	TopToFoundation(Symbol_of_colors, Symbol_of_suits, Symbol_of_numbers);
}

void List::TopToFoundation(char s_color, char s_suit, int s_value){
	List_Node *listtraverse;
	Card_Node *willbemoved = NULL, *cardtraverse, *cardtail, *temptop_head = top_head;

	if(top_head == NULL){
		cout << "Top list is empty, you cannot make this move." << endl;
		return;
	}
		
	cardtraverse = top_head;
	cardtail = top_head;

	while(cardtraverse){
		if(cardtraverse->color == s_color && cardtraverse->suit == s_suit  && cardtraverse->value == s_value){//checking if card's datas are same with user entered datas
			willbemoved = cardtraverse;
			break;
		}
		cardtail = cardtraverse;
		cardtraverse = cardtraverse->next;
	}//end while

	if(willbemoved == NULL){
		cout << "Entered card is not in the top list!" << endl;
		return;
	}

	int number;
	listtraverse = foundation_head;
	if(willbemoved->suit == 'S') number = 0;
	if(willbemoved->suit == 'H') number = 1;
	if(willbemoved->suit == 'D') number = 2;
	if(willbemoved->suit == 'C') number = 3;

	for(int i = 0; i < number; i++)//Moving to foundation list according to card's suit
		listtraverse=listtraverse->next;

	/*Cutting the connection of willbemoved node with other nodes in list*/
	if(cardtraverse == cardtail)//willbemoved node is first node
		temptop_head = temptop_head->next;
	else // willbemoved node is not first node
		cardtail->next = cardtraverse->next;

	if(addcardtolist(listtraverse, willbemoved, 2)){// if movement successful, top_head points second card node
		top_head = temptop_head;
		cout << "Movement is successful!" << endl;
	}
	else{// if not successful
		if(!(cardtraverse == cardtail))// if not first node, the connection between cardtail and willbemoved is recovered
			cardtail->next = willbemoved;
		cout << "Wrong Movement!" << endl;
	}
}

void List::goster_TopToBoard(){// wants input from user for moving card top list to board lis
	char Symbol_of_colors, Symbol_of_suits, tempvalue[4];
	int Symbol_of_numbers, a;
	cout << "Select a card from Top List:";
	scanf("%c %c %s", &Symbol_of_colors, &Symbol_of_suits, tempvalue);

	if(tempvalue[0] == 'A')	Symbol_of_numbers = 1;
	else if(tempvalue[0] == 'J')Symbol_of_numbers = 11;
	else if(tempvalue[0] == 'Q')Symbol_of_numbers = 12;
	else if(tempvalue[0] == 'K')Symbol_of_numbers = 13;
	else 
		sscanf(tempvalue, "%d", &Symbol_of_numbers);

	cout << "Select the number of the destination Board List:";
	scanf("%d", &a);
	TopToBoard(a, Symbol_of_colors, Symbol_of_suits, Symbol_of_numbers);

}

void List::TopToBoard(int listindex, char s_color, char s_suit, int s_value){
	List_Node *listtraverse;
	Card_Node *willbemoved = NULL, *cardtraverse, *cardtail, *temptop_head = top_head;

	if(top_head == NULL){
		cout << "Top list is empty, you cannot make this move." << endl;
		return;
	}
		
	cardtraverse = top_head;
	cardtail = top_head;

	while(cardtraverse){
		if(cardtraverse->color == s_color && cardtraverse->suit == s_suit  && cardtraverse->value == s_value){
			willbemoved = cardtraverse;
			break;
		}
		cardtail = cardtraverse;
		cardtraverse = cardtraverse->next;
	}//end while

	if(willbemoved == NULL){
		cout << "Entered card is not in the top list!" << endl;
		return;
	}

	listtraverse = board_head;
	for(int i = 1; i < listindex; i++)//Moving to board list according to listindex entered by user
		listtraverse = listtraverse->next;

	/*Cutting the connection of willbemoved node with other nodes in list*/
	if(cardtraverse == cardtail)//willbemoved node is first node
		temptop_head = temptop_head->next;
	else // willbemoved node is not first node
		cardtail->next = cardtraverse->next;

	if(addcardtolist(listtraverse, willbemoved, 1)){// if movement successful, top_head points second card node
		top_head = temptop_head;
		cout << "Movement is successful!" << endl;
	}
	else{// if not successful
		if(!(cardtraverse == cardtail))// if not first node, the connection between cardtail and willbemoved is recovered
			cardtail->next = willbemoved;
		cout << "Wrong Movement!" << endl;
	}
}

void List::goster_BoardToBoard(){
	char Symbol_of_colors, Symbol_of_suits, tempvalue[4];
	int Symbol_of_numbers, source, destination;

	cout <<  "Select the number of the source Board List:";
	scanf("%d", &source);
	cout <<  "Select the number of the destination Board List:";
	scanf("%d", &destination);

	cin.ignore(1000, '\n');
	cout <<  "Select a card from the selected source Board List to move:";
	scanf("%c %c %s", &Symbol_of_colors, &Symbol_of_suits, tempvalue);

	if(tempvalue[0] == 'A')	Symbol_of_numbers = 1;
	else if(tempvalue[0] == 'J')Symbol_of_numbers = 11;
	else if(tempvalue[0] == 'Q')Symbol_of_numbers = 12;
	else if(tempvalue[0] == 'K')Symbol_of_numbers = 13;
	else 
		sscanf(tempvalue, "%d", &Symbol_of_numbers);
	BoardToBoard(source, destination, Symbol_of_colors, Symbol_of_suits, Symbol_of_numbers);
}

void List::BoardToBoard(int fromwhere, int towhere, char s_color, char s_suit, int s_value){
	List_Node *sourcelisttraverse = board_head, *targetlisttraverse = board_head;
	Card_Node *willbemoved = NULL, *cardtraverse, *cardtail, *temp_head;

	for(int i = 1; i < fromwhere; i++)// list goes fromwhere times next
		sourcelisttraverse = sourcelisttraverse->next;
	temp_head = sourcelisttraverse->cards;
	cardtraverse = temp_head;
	cardtail = temp_head;
	

	while(cardtraverse){
		if(cardtraverse->isup)// Dont move cards if the entered card is down
			if(cardtraverse->color == s_color && cardtraverse->suit == s_suit  && cardtraverse->value == s_value){
				willbemoved = cardtraverse;
				break;
			}
		cardtail = cardtraverse;
		cardtraverse = cardtraverse->next;
	}//end while

	if(willbemoved == NULL){
		cout << "Wrong Movement!" << endl;
		return;
	}

	for(int i = 1; i < towhere; i++)// list goes towhere times next
		targetlisttraverse = targetlisttraverse->next;

	/*Cutting the connection of willbemoved node with other nodes in list*/
	if(cardtraverse == cardtail)//willbemoved node is first node
		temp_head = NULL;
	else // willbemoved node is not first node
		cardtail->next = NULL;

	if(addcardtolist(targetlisttraverse, willbemoved, 3)){// if movement successful, top_head points second card node
		sourcelisttraverse->cards = temp_head;
		if(!cardtail->isup)// if the card behind of the moved card in source is DOWN, turn it to UP
			cardtail->isup = true;
		cout << "Movement is successful!" << endl;
		return;
	}
	if(!(cardtraverse == cardtail)){// if not first node, the connection between cardtail and willbemoved is recovered
		cardtail->next = willbemoved;
		cout << "Wrong Movement!" << endl;
	}
	
}

void List::goster_BoardToFoundation(){
	int source;
	cout <<  "Select the number of the source Board List:";
	scanf("%d", &source);
	cin.ignore(1000, '\n');
	BoardToFoundation(source);
}

void List::BoardToFoundation(int fromwhere){
	List_Node *sourcelisttraverse = board_head, *targetlisttraverse = foundation_head;
	Card_Node *willbemoved = NULL, *cardtraverse, *cardtail, *temp_head;
	for(int i = 1; i < fromwhere; i++)//goes to list user wants
		sourcelisttraverse = sourcelisttraverse->next;
	temp_head = sourcelisttraverse->cards;
	cardtraverse = temp_head;
	cardtail = temp_head;

	while(cardtraverse->next){// go to last card
		cardtail = cardtraverse;
		cardtraverse = cardtraverse->next;
	}//end while

	willbemoved = cardtraverse;// last card in card list


	int number;
	if(willbemoved->suit == 'S') number = 0;
	if(willbemoved->suit == 'H') number = 1;
	if(willbemoved->suit == 'D') number = 2;
	if(willbemoved->suit == 'C') number = 3;

	for(int i = 0; i < number; i++)//Moving between foundation lists according to card's suit
		targetlisttraverse=targetlisttraverse->next;

	/*Cutting the connection of willbemoved node with other nodes in list*/
	if(cardtraverse == cardtail)//willbemoved node is first node
		temp_head = NULL;
	else // willbemoved node is not first node
		cardtail->next = NULL;

	if(addcardtolist(targetlisttraverse, willbemoved, 4)){// if movement successful, board list's card head points second card node
		sourcelisttraverse->cards = temp_head;
		if(!cardtail->isup)// if the card behind of the moved card in source is DOWN, turn it to UP
			cardtail->isup = true;
		cout << "Movement is successful!" << endl;
		return;
	}
	if(!(cardtraverse == cardtail)){// if moving not successful and card is not first node, the connection between cardtail and willbemoved is recovered
		cardtail->next = willbemoved;
		cout << "Wrong Movement!" << endl;
	}
}

void List::clear_screen(){//Clears the system
	#ifdef _WIN32//If the operation system is windows use "cls"
		std::system("cls");
	#else//for other systems use "clear"
		std::system ("clear");
	#endif
}

void List::close(){//Deletes all linked lists and linkedlists's all nodes
	List_Node *listtraverse;
	Card_Node *cardtraverse;

	/*Deleting top list nodes*/
	cardtraverse = top_head;
	while(top_head){
		cardtraverse = top_head;
		top_head = top_head->next;
		delete cardtraverse;
	}

	/*Deleting board list nodes*/
	listtraverse = board_head;
	while(board_head){
		cardtraverse = board_head->cards;
		while(board_head->cards){
			cardtraverse = board_head->cards;
			board_head->cards = board_head->cards->next;
			delete cardtraverse;
		}
		listtraverse = board_head;
		board_head = board_head->next;
		delete listtraverse;
	}

	/*Deleting foundation list nodes*/
	listtraverse = foundation_head;
	while(foundation_head){
		cardtraverse = foundation_head->cards;
		while(foundation_head->cards){
			cardtraverse = foundation_head->cards;
			foundation_head->cards = foundation_head->cards->next;
			delete cardtraverse;
		}
		listtraverse = foundation_head;
		foundation_head = foundation_head->next;
		delete listtraverse;
	}
	
	fclose(fptr);//Closing reading txt file
}


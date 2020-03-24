#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<signal.h>
#include<errno.h>

struct my_msg{                 /*struktura z przechowujaca wysylane informacje*/
	char nick[16];
	char tekst[255];
	int start;
};

int main(int argc, char* argv[]) {
	if(argc > 1 && argc < 4) /*sprwadzenie ilosci argumentow*/
	{
		int sockfd; /*gniazdo*/
		unsigned int len; /*rozmiar klienta*/
		unsigned int my_port = 7777; /*numer portu*/
		struct sockaddr_in serwer, klient; /*struktury przechowujace informacje o serwerze i kliencie*/
		char ip[20]; /*adres ip*/
		struct my_msg msg, msg2; /*zmienne przechowujace wiadomosci*/
		struct hostent* info; /*zmienna z ktorej otrzymuje ip*/
		int pid; /*pid potomka*/

		strcpy(msg.nick, "NN"); /*przypisanie nicku*/

		if(argc == 3) strcpy(msg.nick, argv[2]);
		msg.nick[strlen(msg.nick)] = '\0';

		sockfd = socket(AF_INET, SOCK_DGRAM, 0); /*tworzenie gniazda*/
		if(sockfd == -1)
    {
      perror("Blad socket");
      exit(errno);
    }

		len = sizeof(klient);

		if((pid = fork()) == 0)
		{
			while(1){
				bzero(&klient, len); /*zerowanie klienta*/
				if(recvfrom(sockfd, &msg2, sizeof(msg), 0, (struct sockaddr*)&klient, &len) != -1) /*otrzymanie wiadomosci*/
				{
					if(msg2.start != 0)
					{
						printf("\n[%s (%s) dolaczyl do rozmowy]\n", msg2.nick, inet_ntoa(klient.sin_addr)); /*wypisanie informacji o dolaczeniu do czatu*/
						write(1, "[", 2);
						write(1, msg.nick, strlen(msg.nick));
						write(1, "]> ", 4);
					}
					else
					{
						if(0 != strcmp(msg2.tekst, "<koniec>"))   /*wypisanie wiadomosci*/
						{
							printf("\n[%s (%s)]> %s\n", msg2.nick,
								inet_ntoa(klient.sin_addr) , msg2.tekst);
							write(1, "[", 2);
							write(1, msg.nick, strlen(msg.nick));
							write(1, "]> ", 4);
						}

					}
				}
				else
				{
					perror("Blad recvfrom!");
					exit(errno);
				}
				if(0 == strcmp(msg2.tekst, "<koniec>")) /*wypisanie informacji o koncu rozmowy*/
				{
					printf("\n[%s (%s) zakonczyl rozmowe]\n", msg2.nick,
						inet_ntoa(klient.sin_addr));
					write(1, "[", 2);
					write(1, msg.nick, strlen(msg.nick));
					write(1, "]> ", 4);
				}
			}
		}
		else
		{
			if((info = gethostbyname(argv[1])) == NULL) /*sprawdzenie czy siec istnieje*/
			{
				herror("Zla nazwa domenowa!");
				exit(errno);
			}
			strcpy(ip, inet_ntoa(*(struct in_addr *)info->h_addr_list[0])); /*wydobycie ip*/
			serwer.sin_family      = AF_INET;           /* IPv4 */
			serwer.sin_addr.s_addr = htonl(INADDR_ANY); /* adres sieci */
			serwer.sin_port        = htons(my_port);    /* port */

			klient.sin_family      = AF_INET; 	    /*IPv4*/
			klient.sin_addr.s_addr = inet_addr(ip);	    /*adres sieci*/
			klient.sin_port	       = htons(my_port);    /*port*/

			if(bind(sockfd, (struct sockaddr *)&serwer, sizeof(serwer))) /*przypisuje adres do gniazda*/
			{
				perror("Blad bind");
				exit(errno);
			}
			printf("Rozpoczynam czat z %s. Napisz <koniec> by zakonczyc czat.\n", ip);
			msg.start = 1;
			if(sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&klient, sizeof(klient)) == -1) /*wyslanie informacji o dolaczeniu do czatu*/
			{
				perror("Blad sendto");
				exit(errno);
			}
			msg.start = 0;
			while(1){
				printf("[%s]> ", msg.nick);
				fgets(msg.tekst, 255, stdin);   /*pobranie wiadomosci od uzytkownika*/
				msg.tekst[strlen(msg.tekst)-1] = '\0';
				if(sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&klient, sizeof(klient)) == -1) /*wyslanie wiadomosci*/
				{
					perror("Blad sendto");
					exit(errno);
				}
				if(0 == strcmp(msg.tekst, "<koniec>")) /*sprawdzenie czy zostal wpisany <koniec>*/
				{
					kill(pid, SIGKILL);	/*zabicie potomka*/
					break;
				}
			}
			wait(NULL);
		}
		close(sockfd);		/*zamkniecie gniazda*/
		return 0;
	}
	else
	{
		printf("Za malo lub za duzo argumentow.\n");
		return 0;
	}
}

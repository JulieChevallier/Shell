#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define BUFFER_SIZE 4096


//*-----affichage personnalisé-----*//
void affichage(){
    char repertoirCourant[1024];
    char nomPc[128];
    char* nomUser;


    nomUser = getlogin();
    gethostname(nomPc,128);
    getcwd(repertoirCourant,1024);

    printf("[Shell]\033[1;32m%s@%s\033[0m:\033[1;34m%s\033[0m$ ", nomUser, nomPc, repertoirCourant);
}

//*-----vérification de sortie-----*//
void siExit(char* buffer){
    if (strcmp("exit",buffer)==0){
            exit(0);
    }
}

//*-----compte le nombre de paramètres-----*//
int nbParametres(char* buffer){
    int compteur=0;
    int e;
    for (e=0;e<strlen(buffer);e++){
        if (buffer[e]==' '){
            ++compteur;
        } 
    }

    return compteur;
}

//*-----copie la commande-----*//
char* copieBuffer(char* buffer, int compteur){
    char *cmd=strdup(buffer);//copie

    return cmd;
}

//*-----sépare les paramètres-----*//
char** separerParametres(char* cmd, int compteur){
    char** argList = malloc(sizeof(char*) * compteur+2);
    char *pch=strtok(cmd," ");//sépare chaine de caractère
        
    int i=0;
    while (pch!=NULL){
        argList[i]=strdup(pch);
        i++;
        pch=strtok(NULL," ");
    }
    argList[i]=NULL;

    return argList;    
}

//*-----vérification si &----*//
int siBackground(char** argList, int compteur){
    int backg=0;
    if(argList[compteur][0] == '&'){//verifie cas &
        argList[compteur] = NULL;//retire l'élément &
        backg = 1; //set TRUE
    }

    return backg;
}

//*-----execute la commande----*//
void executeCommande(char** argList, int compteur){
    int background = siBackground(argList, compteur);

    pid_t pid=fork();//création nouveau processus
    if (pid==0 ){//processus fils
        int att=execvp(argList[0],argList);//remplace l'image du processus actuel par le contenu des arguments + execute
        if (att==-1){//gestion erreur
            printf("%s\n",strerror(errno));
        }
        exit(0);//fin processus fils -> réveil processus père
    }
    if(background==0){//cas &
        waitpid(pid,NULL,NULL);//père attend le fils
    }
}


int main(){
    char buffer[4096];
    while(1){
        affichage();

        fgets(buffer,4096,stdin);
        buffer[strlen(buffer)-1]='\0';

        siExit(buffer);

        int compteur = nbParametres(buffer);
        char *cmd = copieBuffer(buffer, compteur);
        char** argList = separerParametres(buffer, compteur);

        executeCommande(argList,compteur);

        //*-----libération de la mémoire----*//
        int i=0;
        while (argList[i]!=NULL){
            free(argList[i]);
            i++;
        }
        free(cmd);
    }
}

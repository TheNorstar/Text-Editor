#ifndef HEADERFILE_H
#define HEADERFILE_H

// structura pentru managementul mai facil al memoriei
typedef struct Memory{
	void **pointers;
	int size;
}Memory;

// initializarea memoriei
Memory* memory_init(int n){
	Memory *m = malloc(sizeof(Memory));
	m->pointers = malloc(n*sizeof(void*));
	m->size = 0;
	return m;
}

// inserarea in memorie
void push_pointer(Memory *m, void *p){
	m->pointers[m->size] = p;
	m->size++;
}

// eliberarea memoriei
void free_memory(Memory *m){
	int i;
	for(i = 0; i < m->size; i++)
		free(m->pointers[i]);
	free(m->pointers);
	free(m);
}

// definirea unui nod din lista de caractere
typedef struct Text {
    char c;
    struct Text *next;
    struct Text *prev;
}Text;

// definirea listei duble
typedef struct Double_list {
    Text *head;
    Text *tail;
}Double_list;

// initializarea unei nod din lista de caractere
Text* text_init(char c){
    Text *t = malloc(sizeof(Text));
    t->c = c;
    t->next = NULL;
    t->prev = NULL;
    return t;
}

// intializarea listei dublu inlantuite
Double_list* double_list_init(){
    Double_list *d_list = malloc(sizeof(Double_list));
    d_list->head = NULL;
    d_list->tail = NULL;
	return d_list;
}

/* functie ce returneaza un sir de caractere compus din primele k caractere din lista text
   daca k e -1 sirul de caractere contine toate caracterele din lista cu exceptia spatiilor si newline-urilor
*/
char* text_to_str(Text *text,int k){
    Text *it = text;
    int c = 0;
    char *str;
    if (k > 0)
        str = malloc((k+1)*sizeof(char));
    else
        str = malloc(1000*sizeof(char));
    while(it != NULL && c != k){
        str[c++] = it->c;
        it = it->next;
    }
    str[c] = '\0';
    if(k == -1)
    	while(str[c-1] == '\n' || str[c-1] == ' ')
    		c--;
    str[c] = '\0';
    return str;
}

/* inversa functiei de mai sus, returneaza o lista dublu inlantuita
   care reprezinta sirul de caractere primit ca parametru
*/
Text* str_to_text(char *word,Memory *m){
    Text *text = text_init(word[0]);
    Text *start = text;
    Text *it = text;
    int i;
    for(i = 1; word[i] != '\0'; i++){
        text = text_init(word[i]);
        push_pointer(m,(void *) text);
        text->prev = it;
        it->next = text;
        it = it->next;
    }
    return start;
}

// functie ce sterge liniile goale din lista text
void delete_empty_lines(Text *text,int *pos){
	Text *q = text,*p = text->next;
	int x = 1;
	while(p){
		if(q->c == '\n'){
			if(p->c == '\n'){
				if(p->next)
					p->next->prev = q;
				q->next = p->next;
				p = q->next;
				if(x < pos[0])
					pos[0]--;
			}
			else
				x++;
		}
		q = p;
		p = p->next;
	}
}

// functie ce sterge nodul dat din lista
Text* delete_char(Text *text){
    if(text->c == ':')
        return NULL;
    text->prev->next = text->next;
    if(text->next)
        text->next->prev = text->prev;
    return text;
}

// functie ce sterge linia corespunzatoare nodului primit
Text* delete_line(Text *text){
    Text *p = text, *q = text;
    while(p->c != '\n' && p->c != ':')
        p = p->prev;
    while(q->c != '\n' && q->next != NULL)
        q = q->next;
    p->next = q->next;
    if(q->next)
        q->next->prev = p;
    return q;
}

// functie ce sterge k caractere/noduri din lista
Text* delete_k_chars(Text *text, int k){
    Text *it = text;
    Text *temp = text->next;
    int i = 0;
    while(i < k){
        it = it->next;
        i++;
    }
    text->next = it->next;
    if(it->next)
        it->next->prev = text;
    return temp;
}

// functie ce returneaza al y-lea nod de pe linia a x-a
Text* go_to(Text *text,int x, int y){
    int row = 1, col = 0;
    Text *it = text;
    while(it != NULL){
        if(row == x && col == y){
            return it;
        }
        if(it->next != NULL && it->next->c == '\n'){
			if(row == x)
				return it;
            row++;
            col = 0;
        }
        else
            col++;
        it = it->next;
    }
    return NULL;
}

// functie ce returneaza pe a cata pozitie se afla cursorul pe o linie
int get_y(Text *text,Text *cursor,int line){
    int x = 1,y = 0;
    Text *it = text;
    while(it != NULL){
        if(it->c == '\n'){
            x++;
            y = 0;
        }
        if(cursor == it)
            return y;
        it = it->next;
        y++;
    }
    return -1;
}

/* functie ce verifica daca un anumit sir de caractere apare in lista
   se returneaza nodul la care incepe, iar daca nu apare se intoarce NULL
*/
Text* contains(Text *text,char *word){
    int n = strlen(word);
    Text *it = text;
    while(it != NULL){
        if(it->c == word[0]){
            char *str = text_to_str(it,n);
            if(!strcmp(word,str)){
                free(str);
                return it;
            }
            free(str);
        }
        it = it->next;
    }
    return NULL;
}

// functie ce sterge un cuvant din lista
Text* delete_word(Text *text,char *word){
    int n = strlen(word);
    Text *start = contains(text->next,word);
    if(start && start->prev){
        return delete_k_chars(start->prev,n);
    }
    return NULL;
}

// functie ce inlocuieste in lista word1 cu word2
Text* replace_word(Text *text, char *word1, char *word2,Memory *m){
    Text *q = contains(text,word1);
    if(q != NULL){
        Text *new_word = str_to_text(word2,m);
        push_pointer(m,(void *) new_word);
        int i = 0;
        Text *it1 = q;
        while(it1->c == word1[i++])
            it1 = it1->next;
        Text *it2 = new_word;
        while(it2->next != NULL)
            it2 = it2->next;
        it2->next = it1;
        it1->prev = it2;
        new_word->prev = q->prev;
        q->prev->next = new_word;
        return q;
    }
    return NULL;
}

Text* shift(Text *text, int k){
	int i = 0;
	Text *it = text;
	while(it != NULL && i < k){
		it = it->next;
		k++;
	}
	return it;
}
#endif // HEADERFILE_H

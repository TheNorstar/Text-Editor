#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"
#define MAX 10000
int main(){

	// initializarea memoriei
	Memory *memory = memory_init(MAX);

	// deschiderea fisierului editor.in
	FILE *f = fopen("editor.in","r");

	// initializarea listei duble cu santinela pentru text
	Double_list *d_list = double_list_init();
	push_pointer(memory,(void *) d_list);
	Text *sentinel = text_init(':');
	push_pointer(memory,(void *)sentinel);
	d_list->head = sentinel;
	// cursorul folosit la parcurgerea textului
	Text *cursor = d_list->head;
	//stivele pentru comenzile undo/redo
	Command *undo_stack = NULL;
	Command *redo_stack = NULL;

	// pointerul catre inceputul secventei citite
	Text *sequence = NULL;

	// variabile auxiliare
	int sem = 1;
	int lines = 0;
	int seq_len = 0;
	int mode = 0;
	int y = 0;
	// pozitia cursorului
	int *pos = malloc(2*sizeof(int));
	pos[0] = 1;
	pos[1] = 0;

	// citirea se face caracter cu caracter
	char c = fgetc(f);
	// variabila buffer pentru citirea fiecarei linii pe rand
	Double_list *buffer = double_list_init();
	push_pointer(memory,(void *)buffer);
	buffer->head = text_init(c);
	push_pointer(memory,(void *)(buffer->head));
	buffer->tail = buffer->head;
	// iteratorul pentru buffer
	Text *it = buffer->head;

	// se citeste pana la sfarsitul fisierului (EOF)
	while(c != EOF){
		if(c == '\n'){
			// se sterg liniile goale din fisierul editor.in
			delete_empty_lines(d_list->head,pos);
			// la sfarsitul unei linii bufferul este convertit intr-un string normal
			char *line = text_to_str(buffer->head,-1);
			buffer->tail = it;
			lines++;
			/* daca mode are valoarea 0, ne aflam in modul de inserare text,
			   iar daca are valoarea 1, ne aflam in modul de inserare comenzi
			*/
			if(!mode)
				if(!strcmp(line,"::i")){
					// verificam daca inserarea se face la mijloc de linie
					if(cursor->c == '\n' && (lines == 2 || y == 0) ){
						cursor->prev->next = cursor->next;
						cursor = cursor->prev;
						seq_len--;
					}
					// schimbarea in modul de inserare comenzi
					mode = 1;
					// inserarea in stiva de undo
					push(&undo_stack,'w',sequence,pos[0],pos[1],seq_len-3);
					push_pointer(memory,(void *)(undo_stack->pos));
					push_pointer(memory,(void *)undo_stack);
				}

				else{
					// inserarea la inceputul listei cu santinela
					if(cursor->c == ':'){
						buffer->tail->next = d_list->head->next;
						d_list->head->next = buffer->head;
						buffer->head->prev = d_list->head;
						if(!d_list->tail)
							d_list->tail = buffer->tail;
						cursor = buffer->tail;
						if(sem){
							sequence = buffer->head;
							sem = 0;
						}
					}
					// inserarea la pozitia cursorului
					else{
						 if(cursor->next == NULL){
							cursor->next = text_init('\n');
							push_pointer(memory,(void *)(cursor->next));
							cursor->next->prev = cursor;
							cursor = cursor->next;
						}
						buffer->tail->next = cursor->next;
						if(cursor->next)
							cursor->next->prev = buffer->tail;
						cursor->next = buffer->head;
						buffer->head->prev = cursor;
						cursor = buffer->tail;
						if(!cursor->next)
							d_list->tail = cursor;
						if(sem){
							sequence = buffer->head;
							sem = 0;
						}
					}
					// actualizarea pozitiei cursorului
					pos[0]++;
					pos[1] = 0;
				}

			else{
				if(!strcmp(line,"::i")){
					// actualizarea variabilelor auxiliare
					lines = 0;
					mode = 0;
					sem = 1;
					seq_len = 0;
					y = pos[1];
				}
				// salvarea textului
				else if(!strcmp(line,"s")){
					FILE *fw = fopen("editor.out","w");
					Text *it = d_list->head->next;
					// scrierea in fisier a listei
					while(it->next != NULL){
						fputc(it->c,fw);
						it = it->next;
					}
					if(it->c != '\n')
						fputc(it->c,fw);
					fputc('\n',fw);
					// actualizarea stivelor de undo/redo
					undo_stack = NULL;
					redo_stack = NULL;
					fclose(fw);
				}
				// iesirea din program
				else if(!strcmp(line,"q")){
					// eliberarea memoriei & inchiderea fisierului
					free_memory(memory);
					free(line);
					free(pos);
					fclose(f);
					return 0;
				}
				else if(!strcmp(line,"b")){
					Text *temp = cursor;
					cursor = cursor->prev;
					delete_char(temp);
					push(&undo_stack,'t',temp,pos[0],pos[1],1);
					// daca se sterge un newline se actualizeaza pozitia cursorului
					if(temp->c == '\n'){
						pos[0]--;
						pos[1] = get_y(d_list->head,cursor,pos[0]);
					}
					else pos[1]--;
					push_pointer(memory,(void *)(undo_stack->pos));
					push_pointer(memory,(void *)undo_stack);
				}
				else{
					// vector de stringuri pentru argumente/parametrii
					char **args = malloc(2*sizeof(char*));
					push_pointer(memory,(void *)args);
					// parsarea comenzii & parametriilor
					char *command = strtok(line," ");
					char *p = strtok(NULL," ");
					int i = 0;
					while(p){
						args[i++] = p;
						p = strtok(NULL," ");
					}
					while(i < 2)
						args[i++] = NULL;
					//stergerea unei linii
					if(!strcmp(command,"dl")){
						if(args[0]){
							int n = atoi(args[0]);
							Text *q = go_to(d_list->head,n,1);
							int length = 0;
							Text *p = q;
							while(p->next != NULL && p->c != '\n'){
								p = p->next;
								length++;
							}
							q = delete_line(q);
							push(&undo_stack,'t',q,pos[0],pos[1],length+1);
							push_pointer(memory,(void *)(undo_stack->pos));
							push_pointer(memory,(void *)undo_stack);
							if(n == pos[0]){
								cursor = q;
								pos[1] = 0;
							}
							else if(n < pos[0]){
								pos[0]--;
							}
						}
						else{
							Text *q = go_to(d_list->head,pos[0],1);
							int length = 0;
							Text *p = q;
							while(p->next != NULL && p->c != '\n'){
								p = p->next;
								length++;
							}
							push(&undo_stack,'t',q,pos[0],pos[1],length+1);
							push_pointer(memory,(void *)(undo_stack->pos));
							push_pointer(memory,(void *)undo_stack);
							q = delete_line(q);
							cursor = q;
							pos[1] = 0;
						}
					}
					// mutarea cursorului la inceputul unei linii
					else if(!strcmp(command,"gl")){
						int n = atoi(args[0]);
						push(&undo_stack,'p',cursor,pos[0],pos[1],0);
						push_pointer(memory,(void *)(undo_stack->pos));
						push_pointer(memory,(void *)undo_stack);
						cursor = go_to(d_list->head,n,0);
						pos[0] = n;
						pos[1] = 0;
					}
					// mutarea cursorului dupa un anumit caracter
					else if(!strcmp(command,"gc")){
						int n = atoi(args[0]),m;
						push(&undo_stack,'p',cursor,pos[0],pos[1],0);
						push_pointer(memory,(void *)(undo_stack->pos));
						push_pointer(memory,(void *)undo_stack);
						if(args[1]){
							m = atoi(args[1]);
							Text *q = go_to(d_list->head,m,n);
							if(q != NULL)
								cursor = q;
							pos[0] = m;
							pos[1] = n;
						}
						else{
							cursor = go_to(d_list->head,pos[0],n);
							pos[1] = n;
						}
					}
					// stergerea caracterelor de dupa cursor
					else if(!strcmp(command,"d")){
						Text *q;
						int n;
						if(args[0]){
							n = atoi(args[0]);
							q = delete_k_chars(cursor,n);
						}
						else{
							q = delete_k_chars(cursor,1);
							n = 1;
						}
						push(&undo_stack,'t',q,pos[0],pos[1],n);
						push_pointer(memory,(void *)(undo_stack->pos));
						push_pointer(memory,(void *)undo_stack);
					}
					// stergerea unui cuvant
					else if(!strcmp(command,"dw")){
						char *arg = args[0];
						int n = strlen(arg);
						Text *q = delete_word(cursor,arg);
						push(&undo_stack,'t',q,pos[0],pos[1],n);
						push_pointer(memory,(void *)(undo_stack->pos));
						push_pointer(memory,(void *) undo_stack);
					}
					// stergerea tuturor aparitiilor unui cuvant
					else if(!strcmp(command,"da")){
						char *arg = args[0];
						int n = strlen(arg);
						Text *q = delete_word(d_list->head,arg);
						while(q){
							push(&undo_stack,'d',q,pos[0],pos[1],n);
							push_pointer(memory,(void *)(undo_stack->pos));
							push_pointer(memory,(void *) undo_stack);
							q = delete_word(cursor,arg);
						}
					}
					// inlocuirea unui cuvant cu altul
					else if(!strcmp(command,"re")){
						char *arg1 = args[0];
						char *arg2 = args[1];
						int n = strlen(arg1);
						Text *q = replace_word(cursor,arg1,arg2,memory);
						push(&undo_stack,'r',q,pos[0],pos[1],n);
						push_pointer(memory,(void *)(undo_stack->pos));
						push_pointer(memory,(void *) undo_stack);
					}
					// inlocuirea tutoror aparitiilor unui cuvant cu altul
					else if(!strcmp(command,"ra")){
						char *arg1 = args[0];
						char *arg2 = args[1];
						int n = strlen(arg1);
						Text *temp = d_list->head;
						Text *q = replace_word(temp,arg1,arg2,memory);
						if(q)
							temp = q->next;
						while(q){
							push(&undo_stack,'r',q,pos[0],pos[1],n);
							push_pointer(memory,(void *)(undo_stack->pos));
							push_pointer(memory,(void *) undo_stack);
							q = replace_word(temp,arg1,arg2,memory);
							if(q)
								temp = q->next;
						}
					}
					// anularea unei comenzi
					else if(!strcmp(command,"u")){
						if(undo_stack == NULL || undo_stack->text == NULL);
						else if(undo_stack->type == 't'){
							Command *c = pop(&undo_stack);
							Text *q = undo_text(c);
							push(&redo_stack,'t',q,pos[0],pos[1],c->length);
							push_pointer(memory,(void *)(redo_stack->pos));
							push_pointer(memory,(void *) redo_stack);
							pos[0] = c->pos[0];
							pos[1] = c->pos[1];
							cursor = go_to(d_list->head,pos[0],pos[1]);
						}
						else if(undo_stack->type == 'r' || undo_stack->type == 'd'){
							int n = 0;
							Text *q = undo_stack->text;
							while(q->next->prev != q){
								n++;
								q = q->next;
							}
							char *word = text_to_str(undo_stack->text,n);
							Command *c = pop(&undo_stack);
							q = undo_text(c);
							char *temp = text_to_str(undo_stack->text,n);
							while(undo_stack->type == 'r' && strcmp(word,temp) == 0){
								push(&redo_stack,'r',q,pos[0],pos[1],c->length);
								push_pointer(memory,(void *)(redo_stack->pos));
								push_pointer(memory,(void *) redo_stack);
								c = pop(&undo_stack);
								q = undo_text(c);
								free(temp);
								temp = text_to_str(undo_stack->text,n);
							}
							push(&redo_stack,'r',q,pos[0],pos[1],undo_stack->length);
							push_pointer(memory,(void *)(redo_stack->pos));
							push_pointer(memory,(void *) redo_stack);
							free(word);
							free(temp);
						}
						else if(undo_stack->type == 'p'){
							push(&redo_stack,'p',cursor,pos[0],pos[1],0);
							push_pointer(memory,(void *)(redo_stack->pos));
							push_pointer(memory,(void *) redo_stack);
							cursor = undo_pos(pop(&undo_stack),pos);
						}
						else if(undo_stack->type == 'w'){
							cursor = sequence->prev;
							Command *c = pop(&undo_stack);
							push(&redo_stack,'w',sequence,pos[0],pos[1],i-3);
							push_pointer(memory,(void *)(redo_stack->pos));
							push_pointer(memory,(void *) redo_stack);
							delete_k_chars(cursor,c->length-1);
						}
					}
					// refacerea unei comenzi
					else if(!strcmp(command,"r")){
						if(redo_stack == NULL || redo_stack->text == NULL);
						else if(redo_stack->type == 't'){
							Command *c = pop(&redo_stack);
							Text *q = redo_text(c,pos);
							push(&undo_stack,'t',q,pos[0],pos[1],c->length);
							push_pointer(memory,(void *)(undo_stack->pos));
							push_pointer(memory,(void *) undo_stack);
						}
						else if(redo_stack->type == 'r'){
							int n = 0;
							Text *q = redo_stack->text;
							while(q->next->prev != q){
								n++;
								q = q->next;
							}
							char *word = text_to_str(redo_stack->text,n);
							char *word2 = text_to_str(redo_stack->text,n);
							push_pointer(memory,(void *)word);
							q = undo_text(pop(&redo_stack));
							while(redo_stack != NULL && redo_stack->type == 'r' && strcmp(word,word2) == 0){
								free(word2);
								push(&undo_stack,'r',q,pos[0],pos[1],0);
								q = redo_text(pop(&redo_stack),pos);
								word2 = text_to_str(redo_stack->text,n);
							}
							free(word2);
							push(&undo_stack,'r',q,pos[0],pos[1],0);
							push_pointer(memory,(void *)(undo_stack->pos));
							push_pointer(memory,(void *) undo_stack);
						}
						else if(redo_stack->type == 'p'){
							push(&undo_stack,'p',cursor,pos[0],pos[1],0);
							push_pointer(memory,(void *)(undo_stack->pos));
							push_pointer(memory,(void *) undo_stack);
							cursor = undo_pos(pop(&redo_stack),pos);
						}
						else if(redo_stack->type == 'w'){
							Command *c = pop(&redo_stack);
							push(&undo_stack,'w',c->text,pos[0],pos[1],c->length);
							push_pointer(memory,(void *)(undo_stack->pos));
							push_pointer(memory,(void *)undo_stack);
							redo_text(c,pos);
						}
					}
				}
			}
			free(line);
			// citirea primului caracter de pe urmatoarea linie
			c = fgetc(f);
			// reinitializarea bufferului
			buffer->head = text_init(c);
			push_pointer(memory,(void *)(buffer->head));
			buffer->tail = buffer->tail;
			it = buffer->head;
			seq_len++;
		}
		else{
			// citirea urmatorului caracter
			c = fgetc(f);
			// construirea bufferului
			if(c != EOF){
				it->next = text_init(c);
				push_pointer(memory,(void *)(it->next));
				it->next->prev = it;
				it = it->next;
				seq_len++;
			}
		}
	}
	return 0;
}

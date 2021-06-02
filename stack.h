#include "text.h"

// definirea unui nod din stiva undo/redo
typedef struct Command {
    char type;
    Text *text;
    int *pos;
    int length;
    struct Command *next;
}Command;

// initializarea unui element din stiva de comenzi
Command* command_init(char type,Text *text,int x, int y,int length){
    Command *command = malloc(sizeof(Command));
    command->pos = malloc(2*sizeof(int));
    command->type = type;
    command->text = text;
    command->pos[0] = x;
    command->pos[1] = y;
    command->length = length;
    command->next = NULL;
    return command;
}

// inserarea in stiva
void push(Command **Stack,char type,Text *text,int x, int y,int length){
    Command *new_command = command_init(type,text,x,y,length);
    new_command->next = *Stack;
    *Stack = new_command;
}

// eliminarea din stiva
Command* pop(Command **Stack){
    Command *command = *Stack;
    *Stack = (*Stack)->next;
    return command;
}

// anularea unei comenzi de tip text
Text* undo_text(Command *c){
    Text *it = c->text;
    Text *temp = c->text->prev->next;
    while(it->next != NULL && it->next->prev == it){
        it = it->next;
    }
    if(it->next != NULL)
        it->next->prev = it;
    if(c->text->prev)
        c->text->prev->next = c->text;
    if(c->type == 'r')
        return temp;
    else
        return c->text;
}

// anularea unei comenzi de tip pozitie
Text* undo_pos(Command *c,int *pos){
    pos[0] = c->pos[0];
    pos[1] = c->pos[1];
    return c->text;

}

// refacerea unei comenzi de tip text
Text* redo_text(Command *c,int *pos){
    if(c->type == 't' || c->type == 'd'){
        Text *it = c->text;
        int i = 0;
        while(it != NULL && i < c->length){
            i++;
            it = it->next;
        }
        c->text->prev->next = it;
        it->prev = c->text->prev;
        return c->text;
    }
    else if(c->type == 'r'){
        return undo_text(c);
    }
    else if(c->type == 'w'){
        Text *it = c->text;
        shift(it,c->length-1);
        if(it->next)
            it->next->prev = it;
        c->text->prev->next = c->text;
        pos[0] = c->pos[0];
        pos[1] = c->pos[1];
    }
    return NULL;
}

#ifndef _CHARACTER_H
#define	_CHARACTER_H

typedef struct Entity Entity;
typedef struct attack Attack;

int canAttackMe( Entity *victim );
int canAttack( Entity *e );


#endif

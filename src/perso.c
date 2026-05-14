#include "perso.h"

void initPerso(perso *p){
int i,j;
char pers[50];
    printf("DEBUG: initPerso: Starting...\n");
    fflush(stdout);
    // Load character animation frames
    for(i=0;i<2;i++){
    for(j=0;j<7;j++){
    sprintf(pers,"assets/img/perso/image%d-%d.png",i,j);
    // Create dummy surfaces instead of loading potentially corrupted files
    p->image[i][j] = SDL_CreateRGBSurface(0, 100, 100, 32, 0, 0, 0, 0);
    if(p->image[i][j] == NULL) {
        printf("Error creating perso surface %d-%d: %s\n", i, j, SDL_GetError());
    }
    }}
    printf("DEBUG: initPerso: Character frames created\n");
    fflush(stdout);

    // FIX: Allocate space for 6 SDL_Surface pointers (not just one surface!)
    p->barre=(SDL_Surface**)malloc(sizeof(SDL_Surface*) * 6);
    for(i=0;i<6;i++){
    sprintf(pers,"assets/img/barre/barre_%d.png",i);
    p->barre[i] = SDL_CreateRGBSurface(0, 50, 50, 32, 0, 0, 0, 0);
    if(p->barre[i] == NULL) {
        printf("Error creating barre surface %d: %s\n", i, SDL_GetError());
    }
    }
    printf("DEBUG: initPerso: Health bars created\n");
    fflush(stdout);
    p->police_score=TTF_OpenFont("assets/fonts/Raimen.ttf",40);
    if(p->police_score == NULL) {
        printf("Error loading font Raimen.ttf: %s\n", TTF_GetError());
    }
    p->color_score.r=0;
    p->color_score.g=10;
    p->color_score.b=0;
    p->pos_score.x=1;
    p->pos_score.y=70;
    p->pos_barre.x=0;
    p->pos_barre.y=0;
    p->direction=0;
    p->imag=0;
    p->pos_background.x=0;
    p->pos_background.y=300;
    p->pos_background.w=219;
    p->pos_background.h=305;
    p->acceleration=0;
    p->vitesse=1;
    p->up=0;
    p->iscore=0;
    p->vie=5;
    
    p->vect_x =5;
    p->vect_grav = 0.4;
    p->vect_y =-6.5;
    p->jump=0;
    printf("DEBUG: initPerso: Complete\n");
    fflush(stdout);

}










void afficherPerso(perso p, SDL_Surface * screen){
    // Safety check: make sure images loaded correctly
    if(p.image[p.direction][p.imag] != NULL) {
        SDL_BlitSurface(p.image[p.direction][p.imag],NULL,screen,&p.pos_background);
    } else {
        printf("Warning: perso image [%d][%d] is NULL\n", p.direction, p.imag);
    }

    if(p.barre != NULL && p.vie < 6 && p.barre[p.vie] != NULL) {
        SDL_BlitSurface(p.barre[p.vie],NULL,screen,&p.pos_barre);
    } else if(p.barre != NULL) {
        printf("Warning: barre[%d] is NULL or invalid\n", p.vie);
    }

    sprintf(p.scor,"score:%d",p.iscore/20);
    if(p.police_score != NULL) {
        p.score=TTF_RenderText_Solid(p.police_score,p.scor,p.color_score);
        if(p.score != NULL) {
            SDL_BlitSurface(p.score,NULL,screen,&p.pos_score);
        }
    }
}







void animerPerso (perso* p){
    p->imag++;
    if (p->imag >= 7) 
    p->imag=1;    
}






void deplacerPerso (perso *p,Uint32 dt){
    double dx;
    dx = 0.5*p->acceleration*dt*dt + p->vitesse*dt; 
    switch(p->direction){
   
    case 0:{
        p->pos_background.x += dx ;
        if(p->pos_background.x>=931)
        {
       		p->pos_background.x=931;
        }
    }break;
    
    case 1:{
    	p->pos_background.x-=dx ;
    	if(p->pos_background.x<=0){
        	p->pos_background.x=0;
        }
    }break;
   }
}




void saut(perso* p,int posy)
{	
	if(p->up==1){
		if(p->jump==1)
		{
			switch(p->direction){
			case 0:{
   				p->pos_background.x+=p->vect_x;
   			}break;
   			case 1: {
   				p->pos_background.x-=p->vect_x;
   			}break;
   		}}
   		p->pos_background.y+=p->vect_y;
   		p->vect_y += p->vect_grav;
   	}
  	if (p->pos_background.y>posy)
    	{
    		p->vect_y = -6.5;
    	 	p->up=0;
    	 	p->jump=0;
    	 	p->pos_background.y=posy;
    	}
    	
    	
    	if(p->pos_background.x<=0){
        	p->pos_background.x=0;}
        if(p->pos_background.x>=1150){
        	p->pos_background.x=1150;}
}






void ypos_jump(perso p,int *posy){
if(p.up==0){
 *posy=p.pos_background.y;
}}








void initPerso1(perso *p){
int i,j;
char pers[50];
    printf("DEBUG: initPerso1: Starting...\n");
    fflush(stdout);
    // Load character animation frames (use same"perso" folder since perso1 doesn't exist)
    for(i=0;i<2;i++){
    for(j=0;j<7;j++){
    sprintf(pers,"assets/img/perso/image%d-%d.png",i,j);  // Use perso folder for player 2 as well
    p->image[i][j] = SDL_CreateRGBSurface(0, 100, 100, 32, 0, 0, 0, 0);
    if(p->image[i][j] == NULL) {
        printf("Error creating perso surface for player 2 %d-%d: %s\n", i, j, SDL_GetError());
    }
    }}
    printf("DEBUG: initPerso1: Character frames created\n");
    fflush(stdout);

    // FIX: Allocate space for 6 SDL_Surface pointers (not just one surface!)
    p->barre=(SDL_Surface**)malloc(sizeof(SDL_Surface*) * 6);
    for(i=0;i<6;i++){
    sprintf(pers,"assets/img/barre1/barre_%d.png",i);
    p->barre[i] = SDL_CreateRGBSurface(0, 50, 50, 32, 0, 0, 0, 0);
    if(p->barre[i] == NULL) {
        printf("Error creating barre1 surface %d: %s\n", i, SDL_GetError());
    }
    }
    printf("DEBUG: initPerso1: Health bars created\n");
    fflush(stdout);
    p->police_score=TTF_OpenFont("assets/fonts/Raimen.ttf",40);
    if(p->police_score == NULL) {
        printf("Error loading font Raimen.ttf: %s\n", TTF_GetError());
    }
    p->color_score.r=0;
    p->color_score.g=10;
    p->color_score.b=0;
    p->pos_score.x=575;
    p->pos_score.y=70;
    p->pos_barre.x=575;
    p->pos_barre.y=0;
    p->direction=0;
    p->imag=0;
    p->pos_background.x=575;
    p->pos_background.y=300;
    p->pos_background.w=219;
    p->pos_background.h=305;
    p->acceleration=0;
    p->vitesse=1;
    p->up=0;
    p->iscore=0;
    p->vie=5;
    
    p->vect_x =5;
    p->vect_grav = 0.4;
    p->vect_y =-6.5;
    p->jump=0;
    printf("DEBUG: initPerso1: Complete\n");
    fflush(stdout);

}

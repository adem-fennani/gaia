#include "perso.h"

void initPerso(perso *p){
int i,j;
char pers[20];
    for(i=0;i<2;i++){
    for(j=0;j<7;j++){
    sprintf(pers,"perso/image%d-%d.png",i,j);
    p->image[i][j]=IMG_Load(pers);
    }}
    
    p->barre=malloc(sizeof(SDL_Surface));
    for(i=0;i<6;i++){
    sprintf(pers,"barre/barre_%d.png",i);
    p->barre[i]=IMG_Load(pers);
    }
    p->police_score=TTF_OpenFont("Raimen.ttf",40);
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
    
}










void afficherPerso(perso p, SDL_Surface * screen){
    SDL_BlitSurface(p.image[p.direction][p.imag],NULL,screen,&p.pos_background);
     SDL_BlitSurface(p.barre[p.vie],NULL,screen,&p.pos_barre);
      sprintf(p.scor,"score:%d",p.iscore/20);
    p.score=TTF_RenderText_Solid(p.police_score,p.scor,p.color_score);
    SDL_BlitSurface(p.score,NULL,screen,&p.pos_score);
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
char pers[20];
    for(i=0;i<2;i++){
    for(j=0;j<7;j++){
    sprintf(pers,"perso1/image%d-%d.png",i,j);
    p->image[i][j]=IMG_Load(pers);
    }}
    
    p->barre=malloc(sizeof(SDL_Surface));
    for(i=0;i<6;i++){
    sprintf(pers,"barre1/barre_%d.png",i);
    p->barre[i]=IMG_Load(pers);
    }
    p->police_score=TTF_OpenFont("Raimen.ttf",40);
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
    
}

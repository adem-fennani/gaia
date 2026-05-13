#include "menu.h"
#include "perso.h"
#include "minimap.h"


int main()
{
//variables de main menu
	SDL_Surface *screen= NULL; 
	image background,B_play,B_play1, B_settings,B_settings1, B_quit,B_quit1, settings,slayed,exits,exits1;
	// Zero-initialize all image structures
	memset(&background, 0, sizeof(image));
	memset(&B_play, 0, sizeof(image));
	memset(&B_play1, 0, sizeof(image));
	memset(&B_settings, 0, sizeof(image));
	memset(&B_settings1, 0, sizeof(image));
	memset(&B_quit, 0, sizeof(image));
	memset(&B_quit1, 0, sizeof(image));
	memset(&settings, 0, sizeof(image));
	memset(&slayed, 0, sizeof(image));
	memset(&exits, 0, sizeof(image));
	memset(&exits1, 0, sizeof(image));
	SDL_Rect pos_plus,pos_moin;
	int i;
	//char animation[50];//bech n7oto fih asem lta3 image 5ater fil animation kolmara yetbadel el esem 
	
//variables du perso
perso p, p1;
// Zero-initialize the perso structs to avoid garbage values
memset(&p, 0, sizeof(perso));
memset(&p1, 0, sizeof(perso));
printf("DEBUG: Creating game background surface...\n");
fflush(stdout);
image backgame;
memset(&backgame, 0, sizeof(image));
    // TODO: Niv1.png appears to be corrupted (libpng CRC error)
    // For now, creating a dummy surface to test the rest of the code
    backgame.img = SDL_CreateRGBSurface(0, 1150, 650, 32, 0, 0, 0, 0);
    if(backgame.img == NULL) {
        printf("ERROR: Failed to create backgame surface\n");
        fflush(stdout);
    }
    backgame.pos.x=0;
    backgame.pos.y=0;
    backgame.pos.w=1150;
    backgame.pos.h=650;
    
//variables du mini map
	minimap m;
	temps t;
	// Zero-initialize minimap and temps structs
	memset(&m, 0, sizeof(minimap));
	memset(&t, 0, sizeof(temps));
//variables du back
//variables du enemie
    
//Initialize SDL first - MUST happen before using any SDL functions
	SDL_Init(SDL_INIT_EVERYTHING);

//initial video
	putenv("SDL_VIDEO_CENTRED=1=");
	screen= SDL_SetVideoMode(1150,650, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);// fil exmple w1=800,h2=400
	SDL_WM_SetCaption("GAIA REPRESENT ", NULL );//faza:Sets the window tile and icone (null)

	// Initialize SDL_image for PNG loading
	printf("DEBUG: Initializing SDL_image...\n");
	fflush(stdout);
	IMG_Init(IMG_INIT_PNG);
	printf("DEBUG: SDL_image initialized\n");
	fflush(stdout);

	//verif d initial
	if( screen == NULL )
    {
        printf( "Can't set video mode: %s \n", SDL_GetError( ) );
    	return EXIT_FAILURE;
	}

// sound - initialize with optimized settings for SDL 1.2
	// Try smaller buffer size (256) for cleaner audio, use explicit format
	if(Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 256)==-1)
	{
		printf("FAIL AUDIO %s\n",Mix_GetError());
	}
	Mix_Music *music;
	music=Mix_LoadMUS("music.mp3");//load tha music
	if(music == NULL) {
		printf("FAIL MUSIC LOAD %s\n", Mix_GetError());
	}
	Mix_PlayMusic(music,-1);//play music
	
//sound bref - use -1 for all channels, not channel 1
	Mix_Volume(-1, MIX_MAX_VOLUME/3);  // Start with lower volume
	Mix_Chunk *son;
	son = Mix_LoadWAV("mouseclick.wav");
	if(son == NULL) {
		printf("FAIL SOUND LOAD %s\n", Mix_GetError());
	}
	Mix_VolumeChunk(son, MIX_MAX_VOLUME/3);

//background init
printf("DEBUG: Skipping menu image loads for now (testing core game)\n");
fflush(stdout);
// All image loading disabled for testing - we'll use dummy surfaces
//	init_background(&background, "background.jpg");

//MENU BUTTONS
// All button loading disabled
//    init_bouton(&B_play,"B_play.png",&B_settings,"B_settings.png",&B_quit,"B_quit.png");
//    init_bouton(&B_play1,"B_play1.png",&B_settings1,"B_settings1.png",&B_quit1,"B_quit1.png");

//kent fil settings initialisation
// All settings image loading disabled
//    init_background(&settings,"settings.png");
//    init_retour_bouton(&exits,"exits.png");
//    init_retour_bouton(&exits1,"exits1.png");
//    init_volume_slayed(&slayed,"slayed.png");

printf("DEBUG: Core game initialized without images\n");
fflush(stdout);

    pos_moin.x=489;
    pos_moin.y=227;
    pos_moin.w=	512-489;
    pos_moin.h= 235-227;
    
    pos_plus.x=730;
    pos_plus.y=214;
    pos_plus.w=30;
    pos_plus.h=34;
    
//init perso
printf("DEBUG: Initializing TTF...\n");
fflush(stdout);
TTF_Init();
printf("DEBUG: Initializing perso...\n");
fflush(stdout);
initPerso(&p);
printf("DEBUG: Initializing perso1...\n");
fflush(stdout);
initPerso1(&p1);

//iinit minimap
printf("DEBUG: Initializing minimap...\n");
fflush(stdout);
	initmap(&m);
printf("DEBUG: Initializing temps...\n");
fflush(stdout);
	init_temps(&t);
printf("DEBUG: Setup complete, entering main loop...\n");
fflush(stdout);
//event (souris ou clavier)
	SDL_Event event;//evenment 
	int done=1 , n=0;
	//done::: de boucle ken 1 lo3ba te5dem ken 0 means exit
	//s::: hiya tmathel affichage mta3 settings 
	//v::: hiya volume de music
	int k=0,etat=0,s=0,v=64,counterr=0;
	//etat::: nesta3emlouh bech na3emlo test fi ena background mawjoudin ken fi (menu0,play1,setting2,credits3,achivements4)
	//k::: designe le choix selectionnee mta3 menu 
	
// perso
int count_p=0 ,dep=0, acc=0, posy=p.pos_background.y,count_p1=0 ,dep1=0, acc1=0, posy1=p1.pos_background.y,j;
char score[20];
char score1[20];

Uint32 dt, t_prev;

while(done)
{
    dt=SDL_GetTicks()-t_prev;
    t_prev=SDL_GetTicks();
    switch (etat)
    {
        case 0 ://ken ahna mawjoudin fil menu
        {
        	/*affichier_imag(imgBACK[counterr],screen);
		    if(counterr>=20)
		    {
			    counterr=0;
		    }
		    counterr++;*/
		    //SDL_BlitSurface(text,NULL,screen,&POS_txt)
		
		    affichier_imag(background,screen);
		    switch (k)
		    {
             	case 0://k=0 awel mayet7al menu
            	{
            		affichier_imag(B_play,screen);
            		affichier_imag(B_settings,screen);
                	affichier_imag(B_quit,screen);
             	    break;
            	}
            	
            	case 1://k=1 bout play selectionée
            	{
            		affichier_imag(B_play1,screen);
                	affichier_imag(B_settings,screen);
                	affichier_imag(B_quit,screen);
                break;
                }
            	
            	case 2://k=2 menu settings selectionée
            	{
            		affichier_imag(B_play,screen);
                	affichier_imag(B_settings1,screen);
                	affichier_imag(B_quit,screen);
                    break;
		        }
		        case 3:// menu quit selectionée
            	{
            		affichier_imag(B_play,screen);
                	affichier_imag(B_settings,screen);
                	affichier_imag(B_quit1,screen);
		            break;
            	}
            }//fin switch de k
            	
        	while(SDL_PollEvent(&event))//madem fama event yod5el lil boucle 
        	{
            	switch(event.type)
                {
                	case SDL_QUIT:
                    	done = 0;
                    	break;
                    	
                	case SDL_KEYDOWN: 
                	{
                		switch(event.key.keysym.sym)
                		{
                    		case SDLK_UP:
                    		{
                            //houwa fil asel yabda k=0 ki nenzel 3ala fleche up houwa bech ytesti yal9a k=0 bech ybadelha k=2 w yaffichi bouton (quit) mlawna     
                        		if (k==0)
                        		{
                    				k=3;
                    			}

                    			else if (k==1)
                    			{       
                             		k=3;
                        		}

                        		else// les bouton eli fil weseet 
                        		{
                    				k--;
                    			}
                    			
                                break;

                    		}// fin case bout up

                    		case SDLK_DOWN: 
                    		{
                    			if (k==0)
                    				k=1;

                        		else if (k==3)
                        		{
                    				k=1;
                    			}

                        		else
                        		{
                    				k++;
                    			}

                    	    	break;
                            }   //fin case bout down
                    				
                 			case SDLK_RETURN:
                    		{
                        		if(k==1)
                    			{
                    				Mix_PlayChannel(-1,son,0);
                    				etat=1;
                    			}

                        		else if (k==2)
                        		{
                    				Mix_PlayChannel(-1,son,0);
                        				etat=2;
                    			}

                        		else if (k==3)
                        		{
                    				Mix_PlayChannel(-1,son,0);
                    				done=0;//bech yo5rej mel boucle w mil game
                    			}

                    			break;

                    		}   //fin ell case amlt entree 
                    	}   //fin switch de event.key.keysym.sym 

                    	break;
                    }//fin case SDL_KEYDOWN

		            case SDL_MOUSEBUTTONDOWN: 
                    {
                	    if (event.button.button ==SDL_BUTTON_LEFT)//ki tenzel 3ala souris felsa eli 3ala lisar 
                        {
         			        if (event.button.x>B_play.pos.x && event.button.x<B_play.pos.x+B_play.img->w && event.button.y>B_play.pos.y && event.button.y<B_play.pos.y+B_play.img->h)
                        	{  
                        	    Mix_PlayChannel(-1,son,0);
                          	    etat=1;                            			 
                        	}

         			        if (event.button.x>B_settings.pos.x && event.button.x<B_settings.pos.x+B_settings.img->w && event.button.y>B_settings.pos.y && event.button.y<B_settings.pos.y+B_settings.img->h)
                        	{  
                        	    Mix_PlayChannel(-1,son,0);
                          	    etat=2;                            			 
                            }

                        	else if (event.button.x>B_quit.pos.x && event.button.x<B_quit.pos.x+B_quit.img->w && event.button.y>B_quit.pos.y && event.button.y<B_quit.pos.y+B_quit.img->h)//exit
                        	{
                                Mix_PlayChannel(-1,son,0);
                            	done=0; //exit                            		
                        	}
                        }   //if
                    }   //fin case SDL_MOUSEBUTTONDOWN

                    case SDL_MOUSEMOTION: // win enti 9a3ed t7arek fi souris 
                    {                    
                        if (event.button.x>B_play.pos.x && event.button.x<B_play.pos.x+B_play.img->w && event.button.y>B_play.pos.y && event.button.y<B_play.pos.y+B_play.img->h) //play
                        {
                            Mix_PlayChannel(-1,son,0);
                            k=1;
                        }

                        else if (event.button.x>B_settings.pos.x && event.button.x<B_settings.pos.x+B_settings.img->w && event.button.y>B_settings.pos.y && event.button.y<B_settings.pos.y+B_settings.img->h)   //sittings
                        {
                            Mix_PlayChannel(-1,son,0);
                            k=2;
                        }

                        else if (event.button.x>B_quit.pos.x && event.button.x<B_quit.pos.x+B_quit.img->w && event.button.y>B_quit.pos.y && event.button.y<B_quit.pos.y+B_quit.img->h)   //quit
                        {
                            Mix_PlayChannel(-1,son,0);
                            k=3;
                        }

                        else 
                            k=0;

                    }   //fin case SDL_MOUSEMOTION
                }   //finswitch
            }   //fin de boucle de event

            break;

	    }//fin case 0 ;ahna fil menu; switch etat 

	    case 2 ://ken ahna mawjoudin fil settings
	    {
		    affichier_imag(settings,screen);
            switch (s)
            {
                case 0: //awel matoud5el li settings
                {
                    affichier_imag(slayed,screen);
                    affichier_imag(exits,screen);
                    break;
                }

                case 1://affiche bouton back to main menu selectionne
                {
                    affichier_imag(slayed,screen);
                    affichier_imag(exits1,screen);
                    break;
                }
            }   //fin switch s

            while(SDL_PollEvent(&event))
            {
                switch(event.type)
                {
                    case SDL_QUIT://ki tenzel 3ala x 7amra yo5rej 
                        done = 0;
                        break;

                    case SDL_KEYDOWN://event mta3 clavier
                    {
                        switch(event.key.keysym.sym)
                        {
                            case SDLK_ESCAPE://ki tenzel echap tetbadle menu main
                            {
                                etat=0;
                                break;
                            }

                            case SDLK_LEFT://fleche eli 3a lisar 
                            {
                                v-=9;//na9so fil volume mta3 musiq
                                Mix_VolumeMusic(v);
                                slayed.pos.x-=15;//n7arko fi slyde li lysar 
                                if (slayed.pos.x<=515)//ki yosle lil 7ad la5er yo9ef l8adi 
                                    slayed.pos.x=515;
                                    break;
                            }

                            case SDLK_RIGHT://fleche eli 3a limin
                            {
                                v+=9;//nzido fil volume mta3 musiq
                                Mix_VolumeMusic(v);
                                slayed.pos.x+=15;//n7arko fi slyde li limin
                                if (slayed.pos.x>=720)//ki yosle lil 7ad la5er yo9ef l8adi
                                    slayed.pos.x=720;
                                    break;
                            }
                        }//fin de switch event.key.keysym.sym
                    }

                    case SDL_MOUSEBUTTONDOWN://ki tenzel 3ala souris
                    {
                        if (event.button.button ==SDL_BUTTON_LEFT)//ki tenzel 3ala felsa eli 3a lisar 
                        {
                            if(event.button.x>pos_plus.x && event.button.x<pos_plus.x+pos_plus.w && event.button.y>pos_plus.y && event.button.y<pos_plus.y+pos_plus.h)
                            {
                                v+=	10;//nzido fil volume mta3 musiq
                                Mix_VolumeMusic(v);
                                slayed.pos.x+=15;//n7arko fi slyde li limin
                                if (slayed.pos.x>=720)//ki yosle lil 7ad la5er yo9ef l8adi
                                    slayed.pos.x=720;
                            }

                            else if (event.button.x>pos_moin.x && event.button.x<pos_moin.x+pos_moin.w && event.button.y>pos_moin.y && event.button.y<pos_moin.y+pos_moin.h)
                            {
                                v-=10;//na9so fil volume mta3 musiq
                                Mix_VolumeMusic(v);
                                slayed.pos.x-=15;//n7arko fi slyde li lysar 
                                if (slayed.pos.x<=515)//ki yosle lil 7ad la5er yo9ef l8adi 
                                    slayed.pos.x=515;
                            }
                       
                            else if (event.button.x>exits.pos.x && event.button.x<exits.pos.x+exits.img->w && event.button.y>exits.pos.y && event.button.y<exits.pos.y+exits.img->h)
                            {
                                Mix_PlayChannel(-1,son,0);
                                etat=0;
                            }

                            break;
                        }
                    }

                    case SDL_MOUSEMOTION:
                    {
                        if (event.button.x>exits.pos.x && event.button.x<exits.pos.x+exits.img->w && event.button.y>exits.pos.y && event.button.y<exits.pos.y+exits.img->h)
                        {
                            s=1;
                        }

                        else 
                            s=0;
                        // bech nzid ell buttons achivements w credits    
                    }//fin case
                }// fin switch event type
	        }//while sdl pol event taa ell settings
            break;

        }//fin case etat=2 eli ahna fil settings

        case 1://ken hana fil jeux 
        {
            SDL_BlitSurface(backgame.img,NULL,screen,&backgame.pos);
	        afficherPerso(p,screen);
	        afficherPerso(p1,screen);
	        afficherminimap (m, screen);
            SDL_PollEvent(&event);
	        switch(event.type)
            {
        	    case SDL_QUIT:
                    done = 0;
                    break;

                case SDL_KEYDOWN:  
                {
                    switch(event.key.keysym.sym)
                    {
                        case SDLK_ESCAPE://ki tenzel echap tetbadle menu main
                        {
                            etat=0;
                            break;
                        }
                
                        case SDLK_UP:
                    	    p.up=1;
                            break;
                    
                        case SDLK_SPACE:
                            ypos_jump(p,&posy);
		                    p.jump=1;
		                    p.up=1;
		                    break;

                        case SDLK_DOWN:
                            p.pos_background.y+=20;
                            if(p.pos_background.y>=400)
                                p.pos_background.y=400;
                            break;
                    
                        case SDLK_LEFT :
                            p.direction=1;
                            dep=1;
                            break;
                    
                        case SDLK_RIGHT:
                            p.direction=0;
                            dep=1;
                            break;
                    
                        case SDLK_a :
                            acc=1;
                            break;
                    
                        case SDLK_d :
                            acc=2;
                            break;

                        case SDLK_o:
                            p1.up=1;
                            break;

                        case SDLK_j:
                            ypos_jump(p1,&posy1);
                            p1.jump=1;
                            p1.up=1;
                            break;

                        case SDLK_l:
                            p1.pos_background.y+=20;
                            if(p1.pos_background.y>=400)
                                p1.pos_background.y=400;
                            break;

                        case SDLK_k:
                            p1.direction=1;
                            dep1=1;
                            break;

                        case SDLK_m:
                            p1.direction=0;
                            dep1=1;
                            break;
                    }

                    break;
                
                }
            
                case SDL_KEYUP:
                {
                    switch(event.key.keysym.sym)
                    {
                    	case SDLK_LEFT :
                   		dep=0;
                   		p.imag=0;
                   		acc=0;
                    	break;

                    	case SDLK_RIGHT:
                      		dep=0;
                      		p.imag=0;
                      		acc=0;
	                        break;

                    	case SDLK_a :
                    	    acc=0;
                   	        break;

                    	case SDLK_d :
                    	    acc=0;
                    	    break;

                        case SDLK_k:
	                        dep1=0;
	                        p1.imag=0;
	                        acc1=0;
	                        break;

                        case SDLK_m:
	                        dep1=0;
	                        p1.imag=0;
	                        acc1=0;
	                        break;

                        case SDLK_u :
                            acc1=0;
                            break;

                        case SDLK_i :
                            acc1=0;
                            break;

                    }

                	break;
                }
            }
        
            break;
            
        }
    }//fin switch etat

    if(dep==1)
    {
        deplacerPerso (&p,dt);
       	animerPerso (&p);}
        if(acc==1)
        {
            p.acceleration+=0.01;
            if(p.acceleration>=2)
                p.acceleration=2;}
           else
                if(acc==2)
                {
                   	p.acceleration-=0.01;
                   	if(p.acceleration<=-2)
                   		p.acceleration=-2;
                }
                else
               {
               	p.acceleration-=0.05;
               	if(p.acceleration<=0)
               		p.acceleration=0;
                }
                saut(&p,posy);
        
                if(dep1==1)
                {
                    deplacerPerso (&p1,dt);
                    animerPerso (&p1);
                }

                if(acc1==1)
                {
                    p1.acceleration+=0.01;
                    if(p1.acceleration>=2)
                        p1.acceleration=2;}
                    
                    else if(acc1==2)
                    {
                        p1.acceleration-=0.01;
                        if(p1.acceleration<=-2)
                        p1.acceleration=-2;
                    }
        
                    else
                    {
                        p1.acceleration-=0.05;
                        if(p1.acceleration<=0)
                        p1.acceleration=0;
                    }

                    saut(&p1,posy1);
                    SDL_Flip(screen);
                    // Prevent division by zero; cap frame rate at ~30fps minimum
                    if(dt > 0)
                        SDL_Delay(300/dt);
                    else
                        SDL_Delay(16);  // Default ~60fps if dt is 0
		            p.iscore++;
                    if(p.iscore/20>30)
                    {
                        p.vie=5;
                    }

                    if(p.iscore/20>60)
                    {
                        p.vie=4;
                    }

                    if(p.iscore/20>90)
                    {
                        p.vie=3;
                    }

                    if(p.iscore/20>120)
                    {
                        p.vie=2;
                    }

                    if(p.iscore/20>130)
                    {
                        p.vie=1;
                    }

                    p1.iscore++;
                    if(p1.iscore/20>30)
                    {
                        p1.vie=5;
                    }

                    if(p1.iscore/20>60)
                    {
                        p1.vie=4;
                    }

                    if(p1.iscore/20>90)
                    {
                        p1.vie=3;
                    }

                    if(p1.iscore/20>120)
                    {
                        p1.vie=2;
                    }

                    if(p1.iscore/20>130)
                    {
                        p1.vie=1;
                    }

                }//fin while game loop 

                /*lib images
                for (i=0;i<25;i++){
                    librer(imgBACK[i]);
                }
                */  
                librer(B_play);
                librer(B_play1);
                librer(B_settings);
                librer(B_settings1);
                librer(B_quit);
                librer(B_quit1);
    
                librer(settings);
                librer(slayed);
                librer(exits);
                librer(exits1);
	
                //lib sounds
	            Mix_FreeMusic(music); 
	            Mix_FreeChunk( son); 
	
                //perso cleanup
                TTF_CloseFont(p.police_score);
                TTF_CloseFont(p1.police_score);
                TTF_Quit();

                // Free player 1 images
                for (i=0;i<2;i++)
                {
	                for (j=0;j<7;j++)
                    {
		                if(p.image[i][j] != NULL) {
		                    SDL_FreeSurface(p.image[i][j]);
		                }
                    }
                }
                // Free player 1 health bar images
                if(p.barre != NULL) {
                    for(i=0; i<6; i++) {
                        if(p.barre[i] != NULL) {
                            SDL_FreeSurface(p.barre[i]);
                        }
                    }
                    free(p.barre);
                }

                // Free player 2 images
                for (i=0;i<2;i++)
                {
	                for (j=0;j<7;j++)
                    {
		                if(p1.image[i][j] != NULL) {
		                    SDL_FreeSurface(p1.image[i][j]);
		                }
                    }
                }
                // Free player 2 health bar images
                if(p1.barre != NULL) {
                    for(i=0; i<6; i++) {
                        if(p1.barre[i] != NULL) {
                            SDL_FreeSurface(p1.barre[i]);
                        }
                    }
                    free(p1.barre);
                }

                //minimap & background cleanup
                // librer_backg(m);  // This doesn't exist, safe to comment out

                // Cleanup game background image
                if(backgame.img != NULL) {
                    SDL_FreeSurface(backgame.img);
                }

                // Cleanup score surfaces if they exist
                if(p.score != NULL) {
                    SDL_FreeSurface(p.score);
                }
                if(p1.score != NULL) {
                    SDL_FreeSurface(p1.score);
                }

                SDL_Quit();//ll sdl lkoll
                return 0 ;
            }

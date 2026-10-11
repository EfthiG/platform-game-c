#include <stdlib.h>

#include "ADTSet.h"
#include "ADTList.h"
#include "state_alt.h"
#include "../include/set_utils.h"

Object ball_platform;

int compare_rectx(Pointer a, Pointer b){
    Object ob1 = a;
    Object ob2 = b;

    return ob1->rect.x - ob2->rect.x;
}

// Οι ολοκληρωμένες πληροφορίες της κατάστασης του παιχνιδιού.
// Ο τύπος State είναι pointer σε αυτό το struct, αλλά το ίδιο το struct
// δεν είναι ορατό στον χρήστη.
struct state {
	Set objects;			// περιέχει στοιχεία Object (Πλατφόρμες, Αστέρια)
	struct state_info info;	// Γενικές πληροφορίες για την κατάσταση του παιχνιδιού
	float speed_factor;		// Πολλαπλασιαστής ταχύτητς (1 = κανονική ταχύτητα, 2 = διπλάσια, κλπ)
};


// Δημιουργεί και επιστρέφει ένα αντικείμενο

static Object create_object(ObjectType type, float x, float y, float width, float height, VerticalMovement vert_mov, float speed, bool unstable) {
	Object obj = malloc(sizeof(*obj));
	obj->type = type;
	obj->rect.x = x;
	obj->rect.y = y;
	obj->rect.width = width;
	obj->rect.height = height;
	obj->vert_mov = vert_mov;
	obj->vert_speed = speed;
	obj->unstable = unstable;
	return obj;
}

// Προσθέτει αντικείμενα στην πίστα (η οποία μπορεί να περιέχει ήδη αντικείμενα).
// Τα αντικείμενα ξεκινάνε από x = start_x, και επεκτείνονται προς τα δεξιά.
//
// ΠΡΟΣΟΧΗ: όλα τα αντικείμενα έχουν συντεταγμένες x,y σε ένα
// καρτεσιανό επίπεδο.
// - Στο άξονα x το 0 είναι η αρχή της πίστας και οι συντεταγμένες
//   μεγαλώνουν προς τα δεξιά.
//
// - Στον άξονα y το 0 είναι το πάνω μέρος της πίστας, και οι συντεταγμένες μεγαλώνουν
//   προς τα _κάτω_. Η πίστα έχει σταθερό ύψος, οπότε όλες οι
//   συντεταγμένες y είναι ανάμεσα στο 0 και το SCREEN_HEIGHT.
//
// Πέρα από τις συντεταγμένες, αποθηκεύουμε και τις διαστάσεις width,height
// κάθε αντικειμένου. Τα x,y,width,height ορίζουν ένα παραλληλόγραμμο, οπότε
// μπορούν να αποθηκευτούν όλα μαζί στο obj->rect τύπου Rectangle (ορίζεται
// στο include/raylib.h). Τα x,y αναφέρονται στην πάνω-αριστερά γωνία του Rectangle.

static void add_objects(State state, float start_x) {
	// Προσθέτουμε PLATFORM_NUM πλατφόρμες, με τυχαία χαρακτηριστικά.

	for (int i = 0; i < PLATFORM_NUM; i++) {
		Object platform = create_object(
			PLATFORM,
			start_x + 150 + rand() % 80,						// x με τυχαία απόσταση από το προηγούμενο στο διάστημα [150, 230]
			SCREEN_HEIGHT/4 + rand() % SCREEN_HEIGHT/2,			// y τυχαία στο διάστημα [SCREEN_HEIGHT/4, 3*SCREEN_HEIGHT/4]
			i == 0 ? 250 : 50 + rand()%200,						// πλάτος τυχαία στο διάστημα [50, 250] (η πρώτη πάντα 250)
			20,													// ύψος
			i < 3 || rand() % 2 == 0 ? MOVING_UP : MOVING_DOWN,	// τυχαία αρχική κίνηση (οι πρώτες 3 πάντα πάνω)
			0.6 + 3*(rand()%100)/100,							// ταχύτητα τυχαία στο διάστημα [0.6, 3.6]
			i > 0 && (rand() % 10) == 0							// το 10% (τυχαία) των πλατφορμών είναι ασταθείς (εκτός από την πρώτη)
		);
		set_insert(state->objects, platform);

		// Στο 50% των πλατφορμών (τυχαία), εκτός της πρώτης, προσθέτουμε αστέρι
		if(i != 0 && rand() % 2 == 0) {
			Object star = create_object(
				STAR,
				start_x + 200 + rand() % 60,				// x με τυχαία απόσταση από την προηγούμενη πλατφόρμα στο διάστημα [200,260]
				SCREEN_HEIGHT/8 + rand() % SCREEN_HEIGHT/2,	// y τυχαία στο διάστημα [SCREEN_HEIGHT/8, 5*SCREEN_HEIGHT/8]
				30, 30,										// πλάτος, ύψος
				IDLE,										// χωρίς κίνηση
				0,										 	// ταχύτητα 0
				false										// 'unstable' πάντα false για τα αστέρια
			);
			set_insert(state->objects, star);
		}

		start_x = platform->rect.x + platform->rect.width;	// μετακίνηση των επόμενων αντικειμένων προς τα δεξιά
	}
}

// Δημιουργεί και επιστρέφει την αρχική κατάσταση του παιχνιδιού

State state_create() {
	// Δημιουργία του state
	State state = malloc(sizeof(*state));

	// Γενικές πληροφορίες
	state->info.playing = true;				// Το παιχνίδι ξεκινάει αμέσως
	state->info.paused = false;				// Χωρίς να είναι paused.
	state->speed_factor = 1;				// Κανονική ταχύτητα
	state->info.score = 0;				// Αρχικό σκορ 0

	// Δημιουργούμε το vector των αντικειμένων, και προσθέτουμε αντικείμενα
	// ξεκινώντας από start_x = 0.
	state->objects = set_create(compare_rectx, NULL);
	add_objects(state, 0);

	// Δημιουργούμε την μπάλα τοποθετώντας τη πάνω στην πρώτη πλατφόρμα
	Object first_platform = set_node_value(state->objects,set_first(state->objects));
	state->info.ball = create_object(
		BALL,
		first_platform->rect.x,			// x στην αρχή της πλατφόρμας
		first_platform->rect.y - 40,	// y πάνω στην πλατφόρμα
		45, 45,							// πλάτος, ύψος
		IDLE,							// χωρίς αρχική κατακόρυφη κίνηση
		0,								// αρχική ταχύτητα 0
		false							// "unstable" πάντα false για την μπάλα
	);
	ball_platform = first_platform;					
	return state;
}

// Επιστρέφει τις βασικές πληροφορίες του παιχνιδιού στην κατάσταση state

StateInfo state_info(State state) {
	return &state->info;
}

// Επιστρέφει μια λίστα με όλα τα αντικείμενα του παιχνιδιού στην κατάσταση state,
// των οποίων η συντεταγμένη x είναι ανάμεσα στο x_from και x_to.


List state_objects(State state, float x_from, float x_to) {
	List result= list_create(NULL);

    Object a= create_object(PLATFORM, x_from, 10, 10, 10, IDLE, 1, 0);  //φτιαχνουμε δυο τυχαια αντικειμενα με x=x_from και x=x_to αντιστοιχα 
    Object b= create_object(PLATFORM, x_to, 10, 10, 10, IDLE, 1, 0);    //για να τα χρησιμοποιήσουμε ως σημειο αναφορας για να βρουμε στο set τα αντικείμενα που χρειαζόμαστε
    
    for(SetNode node =  set_find_node(state->objects, set_find_eq_or_greater(state->objects, a));
        node != set_find_node(state->objects, set_find_eq_or_smaller(state->objects, b));
        node=set_next(state->objects, node) )
        {
            list_insert_next(result, LIST_BOF, set_node_value(state->objects ,node));
        }
	return result;
}


//Ενημερώνει την κατάσταση state του παιχνιδιού μετά την πάροδο 1 frame.
// Το keys περιέχει τα πλήκτρα τα οποία ήταν πατημένα κατά το frame αυτό.

void state_update(State state, KeyState keys) {
	if(state->info.paused){
		if(keys->p){
			state->info.paused=false;
		}else if(keys->n){
			//Αν το παιχνίδι είναι σε pause και πατηθεί N τότε ενημερώνεται για μόνο 1 frame
			state->info.paused=false;
			state_update(state, keys);
			if(state->info.playing){
				state->info.paused=true;
			}	
		}
	}else if(keys->p){
		state->info.paused=true;
	}

	Object first_object  = create_object(PLATFORM, state->info.ball->rect.x - 200, 10, 10, 10, IDLE, 1, false);
	Object last_object = create_object(PLATFORM, state->info.ball->rect.x + 2 * SCREEN_WIDTH, 10, 10, 10, IDLE, 1, false);

	if(state->info.playing && !state->info.paused){
		state->info.ball->rect.x += (keys->right ? 6 : keys->left ? 1 : 4);	
		//Αν η μπάλα φτάσει στο κάτω μέρος της οθόνης τερματίζει το παιχνίδι
		if(state->info.ball->rect.y >= SCREEN_HEIGHT - state->info.ball->rect.height){
			state->info.playing=false;
			if(state->info.paused){
				state->info.paused = false;
			}
		}
		
		Rectangle recb = state->info.ball->rect;				//βάζουμε σε μία μεταβλητή rec το rectangle της μπαλας για πιο εύκολο έλεγχο
		Object ob;
		if(state->info.ball->vert_mov==IDLE){
			if(recb.x >= ball_platform->rect.x  &&  recb.x <= ball_platform->rect.x + ball_platform->rect.width){  //τα όρια μίας πλατφόρμας είναι το x και το x της πλατφόρμας συν το πλάτος της
				state->info.ball->rect.y = ball_platform->rect.y - state->info.ball->rect.height; //kanoyme thn mpala na akoloythei thn platforma sthn opoia brisketai
			}else{
				if(recb.x > ball_platform->rect.x + ball_platform->rect.width){
					ob = set_find_eq_or_greater(state->objects, state->info.ball); //set_node_value(state->objects, set_next(state->objects, set_find_node(state->objects, ball_platform)));
					while(ob->type != PLATFORM){
						ob = set_node_value(state->objects, set_next(state->objects, set_find_node(state->objects, ob)));
					}
				ball_platform = ob;
			}						// Αν η συντεταγμένη x δεν βρίσκεται στα όρια καμίας πλατφόρμας τότε η μπάλα μπαίνει σε κατάσταση πτώσης (FALLING) με αρχική ταχύτητα 1.5.
				state->info.ball->vert_mov = FALLING;
				state->info.ball->vert_speed = 1.5;
			}
			//Αν βρίσκεται σε ηρεμία (IDLE) και είναι πατημένο το πάνω βέλος μπαίνει σε κατάσταση άλματος με αρχική ταχύτητα 17.
			if(keys->up){
				state->info.ball->vert_mov = JUMPING;
				state->info.ball->vert_speed = 17;
			}          
		}
		
		if(state->info.ball->vert_mov == JUMPING){
			if(recb.x > ball_platform->rect.x + ball_platform->rect.width){
				ob = set_find_eq_or_greater(state->objects, state->info.ball); //set_node_value(state->objects, set_next(state->objects, set_find_node(state->objects, ball_platform)));
				while(ob->type != PLATFORM){
					ob = set_node_value(state->objects, set_next(state->objects, set_find_node(state->objects, ob)));
				}
				ball_platform = ob;
			}		
			state->info.ball->rect.y -= state->info.ball->vert_speed;
			state->info.ball->vert_speed = 0.85 * state->info.ball->vert_speed;
			if(state->info.ball->vert_speed < 0.5){
				state->info.ball->vert_mov = FALLING;
			}
			//An h mpala ftasei sto panw ths othonhs tote na mpainei se katastash FALLING
			if(state->info.ball->rect.y <= 0){
				state->info.ball->vert_mov = FALLING;
			}
		}
		if (state->info.ball->vert_mov == FALLING){
			if(recb.x > ball_platform->rect.x + ball_platform->rect.width){
				ob = set_find_eq_or_greater(state->objects, state->info.ball); //set_node_value(state->objects, set_next(state->objects, set_find_node(state->objects, ball_platform)));
				while(ob->type != PLATFORM){
					ob = set_node_value(state->objects, set_next(state->objects, set_find_node(state->objects, ob)));
				}
				ball_platform = ob;
			}		
			state->info.ball->rect.y += state->info.ball->vert_speed;
			if (state->info.ball->vert_speed<=7){
				state->info.ball->vert_speed += 0.1*state->info.ball->vert_speed;
			}
		}
		for(SetNode node =  set_find_node(state->objects, set_find_eq_or_greater(state->objects, first_object));
        	node != set_find_node(state->objects, set_find_eq_or_smaller(state->objects, last_object));
        	node=set_next(state->objects, node))
			{
			Object obj=set_node_value(state->objects, node);
			if(obj->type==PLATFORM){
				if(obj->vert_mov==MOVING_UP){
					obj->rect.y -= obj->vert_speed;
					if(obj->rect.y<SCREEN_HEIGHT/4){
						obj->vert_mov=MOVING_DOWN;
					}
				}
				else if(obj->vert_mov==MOVING_DOWN){
					obj->rect.y += obj->vert_speed;
					if(obj->rect.y>3*SCREEN_HEIGHT/4){
						obj->vert_mov=MOVING_UP;
					}
				}
				else if(obj->vert_mov==FALLING){
					obj->rect.y += 4;
				}
			}
		}


	//Συγκρούσεις
	if(state->info.ball->vert_mov == FALLING && CheckCollisionRecs(recb, ball_platform->rect)){
		state->info.ball->rect.y = ball_platform->rect.y - state->info.ball->rect.height;
		state->info.ball->vert_mov = IDLE;
		if(ball_platform->unstable){
			ball_platform->vert_mov = FALLING;
		}
	}
	


	for(SetNode node =  set_find_node(state->objects, set_find_eq_or_greater(state->objects, first_object));
        	node != set_find_node(state->objects, set_find_eq_or_smaller(state->objects, last_object));
        	node=set_next(state->objects, node))
			{
			Object obj=set_node_value(state->objects, node);
			if(obj->type == PLATFORM && obj->unstable == true && obj->vert_mov == FALLING && obj->rect.y >= SCREEN_HEIGHT - obj->rect.height){
				set_remove(state->objects, ball_platform);
				break;
			}

			//Αν η μπάλα συγκρουστεί με αστέρι τότε το αστέρι αφαιρείται από το set των αντικειμένων και προστίθενται 10 πόντοι στο σκορ.
			if (obj->type == STAR  &&  CheckCollisionRecs(recb, obj->rect)){	
				state->info.score = state->info.score + 10;
				set_remove(state->objects, obj);		
				break;
			}
	}

		Object obj;
		obj = set_node_value(state->objects ,set_last(state->objects));
		while(obj->type!=PLATFORM){
			obj = set_node_value(state->objects, set_previous(state->objects, set_last(state->objects)));
		}

		if(obj->rect.x - recb.x <= SCREEN_WIDTH){
			add_objects(state, obj->rect.x + obj->rect.width);
			state->speed_factor += 0.1*state->speed_factor;
		}
	}
	//Αν το παιχνίδι έχει τελειώσει και πατηθεί enter, τότε ξαναρχίζει από την αρχή.
	if(!state->info.playing && keys->enter){
		state->info.playing = true;

		//Ftiaxnoyme to state apo thn arxh
		// Γενικές πληροφορίες
		state->info.playing = true;				// Το παιχνίδι ξεκινάει αμέσως
		state->info.paused = false;				// Χωρίς να είναι paused.
		state->speed_factor = 1;				// Κανονική ταχύτητα
		state->info.score = 0;				// Αρχικό σκορ 0

		// Δημιουργούμε το set των αντικειμένων, και προσθέτουμε αντικείμενα
		// ξεκινώντας από start_x = 0.
		state->objects = set_create(compare_rectx, NULL);
		add_objects(state, 0);

		// Δημιουργούμε την μπάλα τοποθετώντας τη πάνω στην πρώτη πλατφόρμα
		Object first_platform = set_find(state->objects , set_node_value(state->objects,set_first(state->objects)));
		state->info.ball = create_object(
			BALL,
			first_platform->rect.x,			// x στην αρχή της πλατφόρμας
			first_platform->rect.y - 40,	// y πάνω στην πλατφόρμα
			45, 45,							// πλάτος, ύψος
			IDLE,							// χωρίς αρχική κατακόρυφη κίνηση
			0,								// αρχική ταχύτητα 0
			false							// "unstable" πάντα false για την μπάλα
		);
		ball_platform = (Object)set_node_value(state->objects, set_first(state->objects));
	}
}


// // Καταστρέφει την κατάσταση state ελευθερώνοντας τη δεσμευμένη μνήμη.

void state_destroy(State state) {
    if (state == NULL)
        return;

    // Free the ball
    free(state->info.ball);

    // Free all objects stored in the set
    set_set_destroy_value(state->objects, free);
    set_destroy(state->objects);

    // Free the game state
    free(state);
}

//////////////////////////////////////////////////////////////////
//
// Test για το state.h module
//
//////////////////////////////////////////////////////////////////

#include "acutest.h"			// Απλή βιβλιοθήκη για unit testing

#include "state_alt.h"

void test_state_create() {

	State state = state_create();
	TEST_ASSERT(state != NULL);

	StateInfo info = state_info(state);
	TEST_ASSERT(info != NULL);

	TEST_ASSERT(info->playing);
	TEST_ASSERT(!info->paused);
	TEST_ASSERT(info->score == 0);

	// Προσθέστε επιπλέον ελέγχους
	TEST_ASSERT(!state_info(state)->ball->unstable);
	TEST_ASSERT(state_info(state)->ball->vert_mov == IDLE);
	
}

void test_state_objects(){
	State state = state_create();
	List objs = state_objects(state, 0, SCREEN_WIDTH);
	TEST_ASSERT(list_size(objs)!=0);
	TEST_ASSERT(objs !=NULL);
}

void test_state_update() {
	State state = state_create();
	TEST_ASSERT(state != NULL && state_info(state) != NULL);

	// Πληροφορίες για τα πλήκτρα (αρχικά κανένα δεν είναι πατημένο)
	struct key_state keys = { false, false, false, false, false, false, false };
	
	// Χωρίς κανένα πλήκτρο, η μπάλα μετακινείται 4 pixels δεξιά
	Rectangle old_rect = state_info(state)->ball->rect;
	state_update(state, &keys);
	Rectangle new_rect = state_info(state)->ball->rect;

	TEST_ASSERT( new_rect.x == old_rect.x + 4 );

	// Με πατημένο το δεξί βέλος, η μπάλα μετακινείται 6 pixels δεξιά
	keys.right = true;
	old_rect = state_info(state)->ball->rect;
	state_update(state, &keys);
	new_rect = state_info(state)->ball->rect;

	TEST_CHECK( new_rect.x == old_rect.x + 6 );
	keys.right=false;
	// Προσθέστε επιπλέον ελέγχους
	// Με πατημένο το αριστερό βέλος, η μπάλα μετακινείται 1 pixel δεξιά
	keys.left = true;
	old_rect = state_info(state)->ball->rect;
	state_update(state, &keys);
	new_rect = state_info(state)->ball->rect;

	TEST_ASSERT( new_rect.x == old_rect.x + 1 );
	keys.left=false;

	//Vertical movement της πλατφόρμας

	//Αν κινείται προς τα πάνω (MOVING_DOWN) μετακινείται προς τα κάτω τόσα pixels όσα η κατακόρυφη ταχύτητά της.
	Object ob;
	List objs = state_objects(state, 0, SCREEN_WIDTH );
	for(ListNode node=list_first(objs) ; node!=LIST_EOF ; node=list_next(objs, node)){
		ob=list_node_value(objs, node);
		if(ob->type == PLATFORM){
			ob->vert_mov = MOVING_DOWN;
			break;
		}
	}
	float old_y = ob->rect.y;
	state_update(state, &keys);
	TEST_ASSERT(ob->rect.y == old_y + ob->vert_speed);

	//Αν περάσει το 3*SCREEN_HEIGHT/4 αλλάζει σε MOVING_DOWN
	ob->rect.y = 3*SCREEN_HEIGHT/4;
	state_update(state, &keys);
	TEST_ASSERT(ob->vert_mov == MOVING_UP);

	//Αν κινείται προς τα πάνω (MOVING_UP) μετακινείται προς τα πάνω τόσα pixels όσα η κατακόρυφη ταχύτητά της.
	ob->vert_mov = MOVING_UP;
	old_y = ob->rect.y;
	state_update(state, &keys);
	TEST_ASSERT(ob->rect.y == old_y - ob->vert_speed);

	// Αν περάσει το SCREEN_HEIGHT/4 αλλάζει σε MOVING_DOWN
	ob->rect.y = SCREEN_HEIGHT/4;
	state_update(state, &keys);
	TEST_ASSERT(ob->vert_mov == MOVING_DOWN);

	//Αν βρίσκεται σε πτώση μετακινείται προς τα κάτω 4 pixels
	ob->vert_mov = FALLING;
	old_y = ob->rect.y;
	state_update(state, &keys);
	TEST_ASSERT(ob->rect.y == old_y + 4);


	//Collisions	
	state_info(state)->playing=true;
	state_info(state)->ball->rect.y = SCREEN_HEIGHT - state_info(state)->ball->rect.height ; 
	state_update(state, &keys);
	TEST_ASSERT(!state_info(state)->playing);

	state_info(state)->playing = true;
	state_info(state)->paused = false;
	//Σύγκρουση μπαλας και αστεριού
	int old_score=state_info(state)->score;
	objs = state_objects(state, 0, 2* SCREEN_WIDTH ); //ελέγχουμε τη λίστα με τα objects από το χ=0 μέχρι το διπλάσιο του SCREEN_WIDTH αφού στην πρώτη οθόνη μπορεί να μην υπάρχει αστέρι για να μπορέσουμε να τεστάρουμε αυτό που θέλουμε
	int old_size=list_size(objs);
	for(ListNode node=list_first(objs) ; node!=LIST_EOF ; node=list_next(objs, node)){
		ob=list_node_value(objs, node);
		if(ob->type==STAR){
			state_info(state)->ball->rect.x = ob->rect.x - ob->rect.width;
			state_info(state)->ball->rect.y = ob->rect.y + (ob->rect.height)/3; //Έστω ότι η μπάλα συγκρούεται με το αστέρι σε αυτό το τυχαίο σημείο του αστεριού
			break;
		}
	}
	state_update(state, &keys);
	objs = state_objects(state, 0, 2 * SCREEN_WIDTH );
	TEST_ASSERT(state_info(state)->score == old_score + 10);    //αν υπάρχει σύγκρουση της μπάλας με αστέρι πρέπει το σκορ να αυξάνεται κατά 10
	TEST_ASSERT(list_size(objs) == old_size -1);                //και το πλήθος των objects να μειώνεται κατά 1
			
	//PLATFORM
	objs = state_objects(state, 0, SCREEN_WIDTH);
	old_size= list_size(objs);
	for(ListNode node=list_first(objs) ; node!=LIST_EOF ; node=list_next(objs, node)){
		ob=list_node_value(objs, node);
		if(ob->type == PLATFORM){
			ob->unstable = true;
			ob->vert_mov = FALLING;
			ob->rect.y = SCREEN_HEIGHT - ob->rect.height -4 ;   //βάζουμε την πλατφόρμα 4 pixel πιο ψηλά αφού όταν καλέσουμε state_update θα μετακινηθέι 4 pixels προς τα κάτω αφού βρίσκεται σε κατάσταση FALLING
			break;
		}
	}
	state_update(state, &keys);
	objs = state_objects(state, 0, SCREEN_WIDTH );
	TEST_ASSERT(list_size(objs) == old_size -1 );


	//Σύγκρουση μπάλας με πλατφόρμα
	state_info(state)->ball->vert_mov = FALLING;
	objs = state_objects(state,	state_info(state)->ball->rect.x - 200 , 2 * SCREEN_WIDTH);
	for(ListNode node=list_first(objs) ; node!=LIST_EOF ; node=list_next(objs, node)){
		ob=list_node_value(objs, node);
		if (ob->type == PLATFORM){
			state_info(state)->ball->rect.y = ob->rect.y - state_info(state)->ball->rect.height + 0.001;
			state_info(state)->ball->rect.x = ob->rect.x + (ob->rect.width)/2;
			break;
		}
	}
	if(ob->vert_mov==MOVING_DOWN){
		state_info(state)->ball->rect.y += ob->vert_speed;
	}else if(ob->vert_mov==MOVING_UP){
		state_info(state)->ball->rect.y -= ob->vert_speed;
	}
	else if(ob->vert_mov==FALLING){
		state_info(state)->ball->rect.y += 4;
	}	
	state_update(state, &keys);
	TEST_ASSERT(state_info(state)->ball->rect.y == ob->rect.y - state_info(state)->ball->rect.height);
	TEST_ASSERT(state_info(state)->ball->vert_mov == IDLE);



}




// Λίστα με όλα τα tests προς εκτέλεση
TEST_LIST = {
	{ "test_state_create", test_state_create },
	{ "test_state_objects", test_state_objects },
	{ "test_state_update", test_state_update},
	
	{ NULL, NULL } // τερματίζουμε τη λίστα με NULL
};
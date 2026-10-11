//////////////////////////////////////////////////////////////////////////////
//
// Παράδειγμα δημιουργίας ενός παιχνιδιού χρησιμοποιώντας τη βιβλιοθήκη raylib
//
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "raylib.h"

#include "interface.h"
#include "state.h"

State state;

void update_and_draw() {
	struct key_state keys;
	keys.down = IsKeyDown(KEY_DOWN);
	keys.enter = IsKeyDown(KEY_ENTER);
	keys.left = IsKeyDown(KEY_LEFT);
	keys.n = IsKeyDown(KEY_N);
	keys.p = IsKeyDown(KEY_P);
	keys.right = IsKeyDown(KEY_RIGHT);
	keys.up = IsKeyDown(KEY_UP);
	state_update(state, &keys);
	interface_draw_frame(state);
}

int main() {
	state = state_create();
	interface_init();

	// Η κλήση αυτή καλεί συνεχόμενα την update_and_draw μέχρι ο χρήστης να κλείσει το παράθυρο
	start_main_loop(update_and_draw);

	interface_close();

	return 0;
}




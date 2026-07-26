package com.bitkovin.shootergame;

import android.view.KeyEvent;
import org.libsdl.app.SDLActivity;

public class GameActivity extends SDLActivity {

    @Override
    protected String[] getLibraries() {
        return new String[] { "main" };
    }

    // SDLActivity's own onBackPressed() only blocks the close if the native
    // SDL_ANDROID_TRAP_BACK_BUTTON hint already reads true at that instant —
    // but that hint is set from inside main(), which only starts running
    // once SDLActivity reaches RESUMED + surface-ready. Overriding here is
    // authoritative from the very first back press, no hint/race involved.
    @Override
    public void onBackPressed() {
        // Forward as a normal key event — same SDLK_AC_BACK your
        // Input::AddAction("back") already listens for — instead of
        // falling through to super.onBackPressed(), which calls finish().
        onNativeKeyDown(KeyEvent.KEYCODE_BACK);
        onNativeKeyUp(KeyEvent.KEYCODE_BACK);
    }
}
// TODO this package must match android:namespace / applicationId in
// app/build.gradle, and this file's path must match the package
// (com/yourstudio/yourgame/GameActivity.java) — rename both together.
package com.bitkovin.shootergame;

import org.libsdl.app.SDLActivity;

/**
 * Thin subclass of SDLActivity — see AndroidManifest.xml, which points the
 * launcher activity at this class rather than at SDLActivity directly, per
 * SDL's own recommendation (README-android.md, "Customizing your
 * application name").
 */
public class GameActivity extends SDLActivity {

    // SDLActivity's default getLibraries() returns {"SDL2", "main"} and
    // expects a standalone libSDL2.so. This project's root CMakeLists.txt
    // forces BUILD_SHARED_LIBS OFF, so SDL2 is linked statically straight
    // into libmain.so instead (see cmake/platforms/android.cmake) — there
    // is no separate libSDL2.so to load, so the default would fail with
    // an UnsatisfiedLinkError. Overriding to just {"main"} is required,
    // not optional, for this project's build.
    @Override
    protected String[] getLibraries() {
        return new String[] {
            "main"
        };
    }
}

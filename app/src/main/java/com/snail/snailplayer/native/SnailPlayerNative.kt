package com.snail.snailplayer.native

class SnailPlayerNative {

    init {
        System.loadLibrary("native-lib")
    }


    external fun _native_setDataSource(url: String)

    external fun _native_prepareAsync()

    external fun _native_start()

    external fun __native_pause()

    external fun _native_setSurface()

    external fun _native_getVideoWidth(): Int

    external fun _native_getVideoHeight(): Int
}
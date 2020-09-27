package com.snail.snailplayer.native

import android.view.Surface
import com.snail.snailplayer.base.SnailPlayerEventCallback

class SnailPlayerNative {

    init {
        System.loadLibrary("native-lib")
    }

    fun setEventCallback(callback: SnailPlayerEventCallback) {
        _native_setEventCallback(callback)
    }

    fun setDataSource(url: String) {
        _native_setDataSource(url)
    }

    fun prepareAsync() {
        _native_prepareAsync()
    }

    fun start() {
        _native_start()
    }

    fun resume() {
        _native_resume()
    }

    fun pause() {
        _native_pause()
    }

    fun setSurface(surface: Surface) {
        _native_setSurface(surface)
    }

    fun getVideoWidth(): Int {
        return _native_getVideoWidth()
    }

    fun getVideoHeight(): Int {
        return _native_getVideoHeight()
    }

    /**
     * 设置回调
     */
    external fun _native_setEventCallback(callback: SnailPlayerEventCallback)

    /**
     * 设置数据源
     */
    external fun _native_setDataSource(url: String)

    /**
     * 异步准备
     */
    external fun _native_prepareAsync()

    /**
     * 开始
     */
    external fun _native_start()

    /**
     * 暂停
     */
    external fun _native_pause()

    /**
     * 继续播放
     */
    external fun _native_resume()

    /**
     * 设置surface
     */
    external fun _native_setSurface(surface: Surface)

    /**
     * 获取视频的宽
     */
    external fun _native_getVideoWidth(): Int

    /**
     * 获取视频的高
     */
    external fun _native_getVideoHeight(): Int
}
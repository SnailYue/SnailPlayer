package com.snail.snailplayer.native

import android.util.Log
import android.view.Surface
import com.snail.snailplayer.base.SnailPlayerEventCallback
import com.snail.snailplayer.base.SnailPlayerTimeListener

class SnailPlayerNative {
    var TAG = SnailPlayerNative::class.java.simpleName

    init {
        System.loadLibrary("native-lib")
    }

    fun setEventCallback(callback: SnailPlayerEventCallback) {
        _native_setEventCallback(callback)
    }

    fun setPlayTimeListener(listener: SnailPlayerTimeListener) {
        _native_setPlayerTimeListener(listener)
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

    fun getDuration(): Long {
        return _native_getDuration()
    }

    fun seekTo(time: Int) {
        Log.d(TAG, "seekTo = $time")
        _native_seek_to(time)
    }

    /**
     * 设置回调
     */
    private external fun _native_setEventCallback(callback: SnailPlayerEventCallback)


    /**
     * 播放时间监听
     */
    private external fun _native_setPlayerTimeListener(listener: SnailPlayerTimeListener)

    /**
     * 设置数据源
     */
    private external fun _native_setDataSource(url: String)

    /**
     * 异步准备
     */
    private external fun _native_prepareAsync()

    /**
     * 开始
     */
    private external fun _native_start()

    /**
     * 暂停
     */
    private external fun _native_pause()

    /**
     * 继续播放
     */
    private external fun _native_resume()

    /**
     * 设置surface
     */
    private external fun _native_setSurface(surface: Surface)

    /**
     * 获取视频的宽
     */
    private external fun _native_getVideoWidth(): Int

    /**
     * 获取视频的高
     */
    private external fun _native_getVideoHeight(): Int

    /**
     * 获取视频的时常
     */
    private external fun _native_getDuration(): Long

    /**
     * 快进、快退
     */
    private external fun _native_seek_to(time: Int)

}
package com.snail.snailplayer

import android.os.Environment
import android.util.Log
import android.view.SurfaceHolder
import com.snail.snailplayer.base.BaseActivity
import com.snail.snailplayer.base.SnailPlayerEventCallback
import com.snail.snailplayer.native.SnailPlayerNative
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : BaseActivity(), SurfaceHolder.Callback {
    val TAG = MainActivity::class.java.simpleName

    override var getView: Int = R.layout.activity_main

    var pathUrl = Environment.getExternalStorageDirectory().path + "/langzhongzhilian.mp4"
    var snailPlayerNative = SnailPlayerNative()

    override fun initView() {
        snailPlayerNative.setDataSource(pathUrl)

        sv_surface.holder.addCallback(this)
        snailPlayerNative.setEventCallback(object : SnailPlayerEventCallback {
            override fun onPrepared() {
                Log.d(TAG, "onPrepared")
                runOnUiThread {
                    var viewWidth = sv_surface.width;
                    var videoWidth = snailPlayerNative.getVideoWidth()
                    var videoHeight = snailPlayerNative.getVideoHeight()
                    var lp = sv_surface.layoutParams
                    lp.width = viewWidth
                    /**
                     * 根据视频的宽高比，以布局的宽为参照，设置视频的高
                     */
                    lp.height = videoHeight / videoWidth * viewWidth
                    sv_surface.layoutParams = lp
                }
                snailPlayerNative.start()
            }
        })
    }


    override fun surfaceCreated(holder: SurfaceHolder?) {
        Log.d(TAG, "surfaceCreated")
        snailPlayerNative.setSurface(holder?.surface!!)
        snailPlayerNative.prepareAsync()
    }

    override fun surfaceChanged(holder: SurfaceHolder?, format: Int, width: Int, height: Int) {
    }

    override fun surfaceDestroyed(holder: SurfaceHolder?) {
    }
}
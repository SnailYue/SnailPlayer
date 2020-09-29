package com.snail.snailplayer

import android.os.Environment
import android.util.Log
import android.view.SurfaceHolder
import android.widget.SeekBar
import com.snail.snailplayer.base.BaseActivity
import com.snail.snailplayer.base.SnailPlayerEventCallback
import com.snail.snailplayer.base.SnailPlayerTimeListener
import com.snail.snailplayer.native.SnailPlayerNative
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : BaseActivity(), SurfaceHolder.Callback {
    val TAG = MainActivity::class.java.simpleName

    override var getView: Int = R.layout.activity_main

    var pathUrl = Environment.getExternalStorageDirectory().path + "/langzhongzhilian.mp4"
    var snailPlayerNative = SnailPlayerNative()
    var seekProgress = 0

    override fun initView() {
        snailPlayerNative.setDataSource(pathUrl)

        sv_surface.holder.addCallback(this)
        snailPlayerNative.setEventCallback(object : SnailPlayerEventCallback {
            override fun onPrepared() {
                Log.d(TAG, "onPrepared")
                runOnUiThread {
                    var viewWidth = sv_surface.width
                    var videoWidth = snailPlayerNative.getVideoWidth()
                    var videoHeight = snailPlayerNative.getVideoHeight()
                    var lp = sv_surface.layoutParams
                    lp.width = viewWidth
                    /**
                     * 根据视频的宽高比，以布局的宽为参照，设置视频的高
                     */
                    lp.height =
                        (((videoHeight.toFloat()) / (videoWidth.toFloat())) * (viewWidth.toFloat())).toInt()
                    sv_surface.layoutParams = lp
                }
                snailPlayerNative.start()
            }
        })

        bt_state?.setOnClickListener {
            if (bt_state.text == "暂停") {
                bt_state.text = "继续"
                snailPlayerNative.pause()
            } else {
                bt_state.text = "暂停"
                snailPlayerNative.resume()
            }
        }
        snailPlayerNative.setPlayTimeListener(object : SnailPlayerTimeListener {

            override fun playTime(current: Double, total: Double) {
                if (currentTime != current.toInt()) {
                    currentTime = current.toInt()
                    totalTime = total.toInt()
                    setProgressBar(currentTime, totalTime)
                    Log.d(TAG, "currentTime = " + current.toInt() + " ,total = " + total.toInt())
                }
            }

            override fun playState(state: Int) {

            }
        })
        sb_bar?.max = 279
        sb_bar?.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {

            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                seekProgress = progress
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {

            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
                snailPlayerNative.seekTo(seekProgress)
            }

        })
    }

    var currentTime = 0
    var totalTime = 0

    private fun setProgressBar(current: Int, totalTime: Int) {
        sb_bar?.max = totalTime
        sb_bar?.setProgress(current)
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
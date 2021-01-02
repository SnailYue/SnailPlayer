package com.snail.snailplayer

import android.Manifest
import android.os.Environment
import android.util.Log
import android.view.SurfaceHolder
import android.widget.SeekBar
import com.hjq.permissions.OnPermission
import com.hjq.permissions.XXPermissions
import com.snail.snailplayer.base.BaseActivity
import com.snail.snailplayer.base.SnailPlayerEventCallback
import com.snail.snailplayer.base.SnailPlayerTimeListener
import com.snail.snailplayer.native.SnailPlayerNative
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : BaseActivity(), SurfaceHolder.Callback {
    val TAG = MainActivity::class.java.simpleName

    override var getView: Int = R.layout.activity_main

    var pathUrl = Environment.getExternalStorageDirectory().path + "/langzhongzhilian.mp4"
    var snailPlayerNative: SnailPlayerNative? = null
    var seekProgress = 0
    var isTouch = false

    override fun initView() {
        XXPermissions.with(this)
            .permission(Manifest.permission.READ_EXTERNAL_STORAGE)
            .permission(Manifest.permission.WRITE_EXTERNAL_STORAGE)
            .request(object : OnPermission {
                override fun hasPermission(granted: MutableList<String>?, all: Boolean) {
                    if (all) {
                        registerPlayer()
                    }
                }

                override fun noPermission(denied: MutableList<String>?, never: Boolean) {
                    System.exit(0)
                }
            })
        bt_state?.setOnClickListener {
            if (bt_state.text == "暂停") {
                bt_state.text = "继续"
                snailPlayerNative?.pause()
            } else {
                bt_state.text = "暂停"
                snailPlayerNative?.resume()
            }
        }
        sb_bar?.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {

            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                if (isTouch) {
                    snailPlayerNative?.seekTo(seekProgress)
                }
                seekProgress = progress
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
                isTouch = true
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
                isTouch = false
            }
        })
    }

    private fun registerPlayer() {
        snailPlayerNative = SnailPlayerNative()
        snailPlayerNative?.setDataSource(pathUrl)
        snailPlayerNative?.setEventCallback(object : SnailPlayerEventCallback {
            override fun onPrepared() {
                Log.d(TAG, "onPrepared")
                runOnUiThread {
                    var viewWidth = sv_surface.width
                    var videoWidth = snailPlayerNative?.getVideoWidth()
                    var videoHeight = snailPlayerNative?.getVideoHeight()
                    var lp = sv_surface.layoutParams
                    lp.width = viewWidth
                    /**
                     * 根据视频的宽高比，以布局的宽为参照，设置视频的高
                     */
                    lp.height =
                        (((videoHeight?.toFloat())!! / (videoWidth?.toFloat()!!)) * (viewWidth.toFloat())).toInt()
                    sv_surface.layoutParams = lp
                }
                snailPlayerNative?.start()
            }
        })
        sv_surface.holder.addCallback(this)
        snailPlayerNative?.setPlayTimeListener(object : SnailPlayerTimeListener {

            override fun playTime(current: Double, total: Double) {
                setProgressBar(current.toInt(), total.toInt())
                Log.d(TAG, "currentTime = " + current.toInt() + " ,total = " + total.toInt())
            }

            override fun playState(state: Int) {
                Log.d(TAG, "state code = " + state)
            }
        })
    }

    private fun setProgressBar(current: Int, totalTime: Int) {
        sb_bar?.max = totalTime
        sb_bar?.progress = current
    }


    override fun surfaceCreated(holder: SurfaceHolder?) {
        Log.d(TAG, "surfaceCreated")
        snailPlayerNative?.setSurface(holder?.surface!!)
        snailPlayerNative?.prepareAsync()
    }

    override fun surfaceChanged(holder: SurfaceHolder?, format: Int, width: Int, height: Int) {
    }

    override fun surfaceDestroyed(holder: SurfaceHolder?) {
    }
}
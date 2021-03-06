package com.snail.snailplayer.base

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity

abstract class BaseActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(getView)
        initView()
    }

    abstract var getView: Int

    abstract fun initView()

    open fun loadData() {}

}
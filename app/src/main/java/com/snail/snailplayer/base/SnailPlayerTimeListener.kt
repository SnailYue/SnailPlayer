package com.snail.snailplayer.base

interface SnailPlayerTimeListener {

    fun playTime(current: Double, total: Double)

    fun playState(state: Int)
}
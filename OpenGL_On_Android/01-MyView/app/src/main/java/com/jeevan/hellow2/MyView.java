package com.jeevan.hello2;

import android.content.Context;
import android.graphics.Color;
import android.widget.TextView;
import android.view.Gravity;

public class MyView extends TextView
{
	MyView(Context context){
			super(context);
			setTextColor(Color.rgb(255,128,0));
			setText("HelloTwoWorld!");
			setTextSize(60);
			setGravity(Gravity.CENTER);
	}
}
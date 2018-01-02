package com.jeevan.hello1;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;
import android.view.Window;
import android.view.WindowManager;
import android.content.pm.ActivityInfo;
import android.view.Gravity;
import android.graphics.Color;

public class MainActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //setContentView(R.layout.activity_main);
		this.requestWindowFeature(Window.FEATURE_NO_TITLE); //remove title bar
		
		//Fullscreen
		this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
		
		//Force activity window for landscape
		MainActivity.this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
		
		//Set window background color to black
		TextView myTextView = new TextView(this);
			
		myTextView.setText("HelloOneWorld!");
		myTextView.setTextSize(60);
		myTextView.setTextColor(Color.GREEN);
		myTextView.setGravity(Gravity.CENTER);
		
		setContentView(myTextView);
		
		getWindow().getDecorView().setBackgroundColor(Color.rgb(0,0,0));
		
    }
}

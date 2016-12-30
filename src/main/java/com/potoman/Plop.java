package com.potoman;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.nio.Buffer;
import java.nio.ByteBuffer;

import com.sun.javafx.image.impl.ByteRgb;

import javafx.application.Application;
import javafx.stage.Stage;
import javafx.scene.*;
import javafx.scene.paint.*;
import javafx.scene.canvas.*;
import javafx.scene.image.Image;
import javafx.scene.image.PixelFormat;
import javafx.scene.image.PixelWriter;


@SuppressWarnings("restriction")
public class Plop extends Application {

	private PixelWriter mPW = null;
    
    public final native String sayHello();
    public final native int timesHello(int x, int y);
    public final native boolean initCanvas(Canvas canvas);
    
	public static void main(String[] args) {
		System.load("C:\\Users\\Maxime\\Documents\\Visual Studio 2012\\Projects\\gstreamer-jni-javafx\\Debug\\gstreamer-jni-javafx.dll");
		
        launch(args);
    }

	public void notifyFrame(byte[] frame) {
        System.out.println("notifyFrame");
		mPW.setPixels(0, 0, 320, 240, PixelFormat.getByteRgbInstance(), frame, 0, 320 * 3);
	}

	public void notifyPlop() {
        System.out.println("plop");
	}
	
	@Override
    public void start(Stage primaryStage) {
		System.out.println(sayHello());
        primaryStage.setTitle("Hello World!");
        Group root = new Group();
		Scene s = new Scene(root, 600, 600, Color.BLACK);

		final Canvas canvas = new Canvas(320, 240);
		final GraphicsContext gc = canvas.getGraphicsContext2D();
//		Image img;
//		try {
//			img = new Image(new FileInputStream(new File("C:\\Users\\Maxime\\Pictures\\14725461_10157565888300247_6468498154741700334_n.jpg")));
//			gc.drawImage(img, 0, 0);
//		} catch (FileNotFoundException e) {
//			e.printStackTrace();
//		}
//		img = new Image
//		img = JavaFXImageConversion.getJavaFXImage(new byte[] {(byte) 255, 0, 0}, 1, 1);
//		gc.drawImage(img, 0, 0);
//		ByteRgb pf;
//		<ByteBuffer>. pf = new PixelFormat<ByteBuffer>
//		new Thread(new Runnable() {
//			
//			public void run() {
//				try {
//					Thread.sleep(3000);
//				} catch (InterruptedException e) {
//					// TODO Auto-generated catch block
//					e.printStackTrace();
//				}
//				byte[] data = new byte[] {0, (byte) 0xFF, 0};
//				gc.getPixelWriter().setPixels(0, 0, 1, 1, PixelFormat.getByteRgbInstance(), data, 0, 1);
//			}
//		}).start();
		
		
		//setFill(Color.BLUE);
		//gc.fillRect(75,75,100,100);
		 
		root.getChildren().add(canvas);
        primaryStage.setScene(s);
        primaryStage.show();
        
        System.out.println("plop");
        
        try {
			Thread.sleep(1000);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
        
        byte[] data = new byte[230400];
        for (int index = 0; index < 230400; index += 3) {
        	data[index] = (byte) 0xFF;
        	data[index + 1] = 0;
        	data[index + 2] = 0;
        }
		mPW = gc.getPixelWriter();
		mPW.setPixels(0, 0, 320, 240, PixelFormat.getByteRgbInstance(), data, 0, 320 * 3);

        try {
			Thread.sleep(1000);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		initCanvas(canvas);
    }
}


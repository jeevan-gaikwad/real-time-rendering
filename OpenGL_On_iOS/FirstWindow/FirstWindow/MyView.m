#import "MyView.h"
@implementation MyView
{
	NSString *centralText;
}

- (id) initWithFrame:(CGRect)frameRect
{
    self=[super initWithFrame:frameRect];
	if(self)
	{
		[self setBackgroundColor:[UIColor whiteColor]];
		centralText=@"Hello World!!";
		
		//Gesture recognition
		
		UITapGestureRecognizer *singleTapGestureRecognizer=
		[[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(onSingleTap:)];
		[singleTapGestureRecognizer setNumberOfTapsRequired:1];
		[singleTapGestureRecognizer setNumberOfTouchesRequired:1];
		[singleTapGestureRecognizer setDelegate:self];
		[self addGestureRecognizer :singleTapGestureRecognizer];
		
		UITapGestureRecognizer *doubleTapGestureRecognizer=[[UITapGestureRecognizer alloc]initWithTarget:self action:
		@selector(onDoubleTap:)];
		[doubleTapGestureRecognizer setNumberOfTapsRequired:2];
		[doubleTapGestureRecognizer setNumberOfTouchesRequired:1];
		[doubleTapGestureRecognizer setDelegate:self];
		[self addGestureRecognizer:doubleTapGestureRecognizer];
		
		[singleTapGestureRecognizer requireGestureRecognizerToFail:doubleTapGestureRecognizer];
		
		//Swipe gesture
		UISwipeGestureRecognizer *swipeGestureRecognizer=[[UISwipeGestureRecognizer alloc]initWithTarget:self action:
		@selector(onSwipe:)];
		[self addGestureRecognizer:swipeGestureRecognizer];
		//long press gesture
		UILongPressGestureRecognizer *longPressGestureRecognizer=[[UILongPressGestureRecognizer alloc]initWithTarget:self action:
		@selector(onLongPress:)];
		[self addGestureRecognizer:longPressGestureRecognizer];
	}
	return (self);
}

//Only override draw rect
- (void)drawRect:(CGRect)rect
{
	//Black background
	UIColor *fillColor=[UIColor blackColor];
	[fillColor set];
	UIRectFill(rect);
	
	//Dictionary with KVC
	NSDictionary *dictionaryForTextAttributes=[NSDictionary dictionaryWithObjectsAndKeys:[UIFont fontWithName:@"Helvetica" size:24],
	 NSFontAttributeName, [UIColor greenColor], NSForegroundColorAttributeName, nil];
	 
	 CGSize textSize=[centralText sizeWithAttributes:dictionaryForTextAttributes];
	 CGPoint point;
	 point.x=(rect.size.width/2)-(textSize.width/2);
	 point.y=(rect.size.height/2)-(textSize.height/2)+12;
	 
    [centralText drawAtPoint:point withAttributes:dictionaryForTextAttributes];
}

//Become first responder
-(BOOL) acceptsFirstResponder
{
	//code
	return (YES);
}

- (void) touchesBegan: (NSSet *)touches withEvent:(UIEvent *) event
{
	
}

-(void)onSingleTap:(UITapGestureRecognizer *) gr
{
	centralText = @"'onSingleTap' Event Occured";
	[self setNeedsDisplay];
}

-(void) onDoubleTap:(UITapGestureRecognizer *)gr
{
	centralText = @"'onDoubleTap' Event occured";
	[self setNeedsDisplay];
}

-(void)onSwipe:(UISwipeGestureRecognizer *)gr
{
	[self release];
	exit(0);
}

-(void)onLongPress:(UILongPressGestureRecognizer *)gr
{
	centralText = @"'OnLongPress' Event occured";
	[self setNeedsDisplay];
}

-(void)dealloc
{
	[super dealloc];
}

@end

//interface impl

#import "AppDelegate.h"
#import "GLView.h"

@implementation AppDelegate
{
@private
    NSWindow *window;
    GLView *view;
}


- (void) applicationDidFinishLaunching:(NSNotification *)aNotification
{
    //Code for log file
    NSBundle *mainBundle = [NSBundle mainBundle];
    NSString *appDirName = [mainBundle bundlePath];
    NSString *parentDirPath = [appDirName stringByDeletingLastPathComponent];
    NSString *logFileNameWithPath = [NSString stringWithFormat:@"%@/Log.txt",parentDirPath];
    const char *pszLogFileNameWithPath = [logFileNameWithPath cStringUsingEncoding:NSASCIIStringEncoding];
    //gpFile=fopen(pszLogFileNameWithPath,"w");
    gpFile=fopen("Log.txt","w");
    if(gpFile==NULL)
    {
        printf("Can not create log file. \n Exiting...\n");
        [self release];
        [NSApp terminate:self];
    }
    //file created successfully
    fprintf(gpFile, "Program is started successfully");
    
    //window code®
    NSRect win_rect;
    win_rect = NSMakeRect(0.0,0.0,800.0,600.0);
    
        //create simple window
    window = [[NSWindow alloc] initWithContentRect:win_rect
                                         styleMask:NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|
              NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskResizable
                                           backing:NSBackingStoreBuffered defer:NO];
              [window setTitle:@"Jeevan's first Window on Mac"];
    [window center];
    view=[[GLView alloc]initWithFrame:win_rect];
    
    [window setContentView:view];
    [window setDelegate:self];
    [window makeKeyAndOrderFront:self];
    
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
    fprintf(gpFile, "Program terminated successfully.");
    if(gpFile)
    {
        fclose(gpFile);
        gpFile=NULL;
    }
    
}


- (void)windowWillClose:(NSNotification *)notification
{
        //code
    [NSApp terminate:self];
}

-(void) dealloc
{
       //code
    [view release];
    [window release];
    [super dealloc];
}

@end



//
//  main.m
//  
//
//  Created by Jeevan Gaikwad on 01/06/18.
//

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import "AppDelegate.h"

FILE *gpFile=NULL;
//entry function main
int main(int argc, const  char *argv[])
{
    NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc]init];
    NSApp = [NSApplication sharedApplication];
    [NSApp setDelegate:[[AppDelegate alloc]init]];
    [NSApp run];
    
    [pPool release];
    printf("In main");    return (0);
}



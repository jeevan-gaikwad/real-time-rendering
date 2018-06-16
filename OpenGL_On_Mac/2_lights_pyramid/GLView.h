#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#import <QuartzCore/CVDisplayLink.h>
#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>
#import "vmath.h"

CVReturn MyDisplayLinkCallback(CVDisplayLinkRef, const CVTimeStamp*, const CVTimeStamp*,
                               CVOptionFlags, CVOptionFlags *, void *);

enum{
    JCG_ATTRIBUTE_VERTEX = 0,
    JCG_ATTRIBUTE_COLOR,
    JCG_ATTRIBUTE_NORMAL,
    JCG_ATTRIBUTE_TEXTURE0
};

@interface GLView : NSOpenGLView
@end



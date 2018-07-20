//
//  MyView.h
//  FirstWindow
//
//  Created by Jeevan on 10/07/18.
//

#import <UIKit/UIKit.h>

enum{
    JCG_ATTRIBUTE_VERTEX = 0,
    JCG_ATTRIBUTE_COLOR,
    JCG_ATTRIBUTE_NORMAL,
    JCG_ATTRIBUTE_TEXTURE0
};
@interface GLESView : UIView <UIGestureRecognizerDelegate>
-(void)startAnimation;
-(void)stopAnimation;
@end

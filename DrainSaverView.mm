#import "DrainSaverView.h"
#import "timedrain.cpp"


@implementation DrainSaverView

- (id)initWithFrame:(NSRect)frame isPreview:(BOOL)isPreview
{
    self = [super initWithFrame:frame isPreview:isPreview];
    if (self) {
 
		NSOpenGLPixelFormatAttribute attribs[] =
        {	NSOpenGLPFAAccelerated,
			NSOpenGLPFADepthSize, 16,
			//NSOpenGLPFAMinimumPolicy,
			//NSOpenGLPFAClosestPolicy,
			0
        };
		
		NSOpenGLPixelFormat *format = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attribs] autorelease];
		
        _view = [[[NSOpenGLView alloc] initWithFrame:NSZeroRect pixelFormat:format] autorelease];
        [self addSubview:_view];
    }
    return self;
}

- (void)dealloc
{
	[_view removeFromSuperview];
    [super dealloc];
}

- (void)setFrameSize:(NSSize)newSize
{
    [super setFrameSize:newSize];
    [_view setFrameSize:newSize];
    _initedGL = NO;	
}

- (void)startAnimation
{
    [super startAnimation];
}

- (void)stopAnimation
{
    _initedGL = NO;
    [super stopAnimation];
    return;
}

- (void)drawRect:(NSRect)rect
{
	renderScene();
    [super drawRect:rect];
}

- (void)animateOneFrame
{
    [[_view openGLContext] makeCurrentContext];
    if (!_initedGL) {
        resizeWindow([self frame].size.width, [self frame].size.height);
		initGL();
        _initedGL = YES;
    } else {
        renderScene();
    }
    return;
}

- (BOOL)hasConfigureSheet
{
    return NO;
}

- (NSWindow*)configureSheet
{
    return nil;
}

@end

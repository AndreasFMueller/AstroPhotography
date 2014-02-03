//
//  CcdSelectionController.h
//  IceStar
//
//  Created by Andreas Müller on 02.02.14.
//  Copyright (c) 2014 Andreas Müller. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "camera.h"

@interface CcdSelectionController : UITableViewController {
    snowstarCameraPrx  *camera;
}

- (id)initWithCamera: (snowstarCameraPrx *)_camera;

@end

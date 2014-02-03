//
//  BinningController.h
//  IceStar
//
//  Created by Andreas Müller on 02.02.14.
//  Copyright (c) 2014 Andreas Müller. All rights reserved.
//

#import <UIKit/UIKit.h>
#include <camera.h>

@interface BinningController : UITableViewController {
    snowstarBinningSet *binningset;
}

- (id)initWithBinningSet: (snowstarBinningSet *)_binningset;
- (snowstarBinningMode*)modeAtIndex:(int)index;

@end

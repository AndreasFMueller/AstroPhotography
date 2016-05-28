//
//  InstrumentsServiceController.h
//  IceStar
//
//  Created by Andreas Müller on 28.05.16.
//  Copyright © 2016 Andreas Müller. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "InstrumentsServiceController.h"
#import "ServiceTableViewController.h"

@interface InstrumentsServiceController : ServiceTableViewController<UITableViewDelegate>

@property (strong,readwrite) id instrumentsViewController;

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath;

@end

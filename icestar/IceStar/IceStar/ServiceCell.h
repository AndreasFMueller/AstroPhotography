//
//  ServiceCell.h
//  IceStar
//
//  Created by Andreas Müller on 28.05.16.
//  Copyright © 2016 Andreas Müller. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "ServerInfo.h"

@interface ServiceCell : UITableViewCell {
    UILabel *nameLabel;
    UILabel *addressLabel;
    UILabel *devicesLabel;
    UILabel *imagesLabel;
    UILabel *repositoryLabel;
    UILabel *instrumentsLabel;
    UILabel *guidingLabel;
    UILabel *focusingLabel;
    UILabel *tasksLabel;
}

@property (nonatomic,strong) ServerInfo *serverinfo;

- (id)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier;
- (void)setServerinfo: (ServerInfo *)serverinfo;
- (void)updateState;
- (void)setSelected:(BOOL)selected animated:(BOOL)animated;

@end

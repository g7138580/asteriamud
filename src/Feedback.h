#ifndef FEEDBACK_H
#define FEEDBACK_H

typedef struct feedback_struct FEEDBACK;

#include "Global/List.h"

enum FeedbackType
{
	FEEDBACK_BUG,
	FEEDBACK_IDEA,
	FEEDBACK_TYPO
};

struct feedback_struct
{
	char		*name;
	char		*message;
	char		*room_id;
	int			type;
	time_t		time_stamp;
};

extern LIST *Feedback;

extern void OverwriteFeedback( void );
extern void SaveFeedback( FEEDBACK *feedback );
extern void LoadFeedback( void );
extern FEEDBACK *NewFeedback( void );
extern void DeleteFeedback( FEEDBACK *feedback );

#endif

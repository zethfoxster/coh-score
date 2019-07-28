#pragma once

typedef struct
{
	int				allocType;
	void*			pData;
	size_t			allocSize;
	int				blockType;
	long			request_id;
	unsigned int	stack_id;
	unsigned int	thread_id;
} MemTrackEvent;

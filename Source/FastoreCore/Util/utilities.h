#pragma once

inline int boolToNormalizedInt(bool forward)
{
	return forward * 2 - 1;
}

inline int boolToInt(bool forward)
{
	return forward * forward;
}
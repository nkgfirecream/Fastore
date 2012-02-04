#pragma once

inline int boolToNormalizedInt(bool forward)
{
	return forward * 2 - 1;
}

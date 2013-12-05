#ifndef __MOCK_APP_USER_H_
#define __MOCK_APP_USER_H_

#include <opendnp3/AppInterfaces.h>

#include <sstream>

namespace opendnp3
{

// @section desc Test class for app layer
class MockAppUser : public IAppUser
{
public:

	struct State {
		friend std::ostream& operator<<(std::ostream& output, const State& s);

		State();

		bool operator==(const State& arState) const;

		size_t NumLayerUp;
		size_t NumLayerDown;
		size_t NumUnsol;
		size_t NumSolSendSuccess;
		size_t NumSolFailure;
		size_t NumUnsolSendSuccess;
		size_t NumUnsolFailure;
		size_t NumPartialRsp;
		size_t NumFinalRsp;
		size_t NumRequest;
		size_t NumUnknown;
	};

	MockAppUser(bool aIsMaster);

	// Implement IAppUser
	void OnLowerLayerUp();
	void OnLowerLayerDown();

	void OnSolSendSuccess();
	void OnSolFailure();

	void OnUnsolSendSuccess();
	void OnUnsolFailure();

	bool IsMaster();
	void OnPartialResponse(const APDU&);
	void OnFinalResponse(const APDU&);
	void OnUnsolResponse(const APDU&);
	void OnRequest(const APDU&, SequenceInfo);
	void OnUnknownObject();

	bool Equals(const State& arState) const;

private:

	bool mIsMaster;

public:
	State mState;
};

}

#endif


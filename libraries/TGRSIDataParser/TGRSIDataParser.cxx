#include "TGRSIDataParser.h"
#include "TGRSIDataParserException.h"

#include "TChannel.h"
#include "Globals.h"

#include "TScalerQueue.h"

#include "TEpicsFrag.h"
#include "TParsingDiagnostics.h"

#include "Rtypes.h"

#include "TMidasEvent.h"
#include "TXMLOdb.h"
#include "TRunInfo.h"
#include "TFragment.h"
#include "TBadFragment.h"

TGRSIDataParser::TGRSIDataParser()
   : TDataParser()
{
	fState = EDataParserState::kGood;
	fIgnoreMissingChannel = TGRSIOptions::Get()->IgnoreMissingChannel();
}

TGRSIDataParser::~TGRSIDataParser()
{
}

int TGRSIDataParser::Process(std::shared_ptr<TRawEvent> rawEvent)
{
	std::shared_ptr<TMidasEvent> event = std::static_pointer_cast<TMidasEvent>(rawEvent);
   int   banksize;
   void* ptr;
   int   frags = 0;
   try {
      switch(event->GetEventId()) {
      case 1:
         event->SetBankList();
         if((banksize = event->LocateBank(nullptr, "WFDN", &ptr)) > 0) {
            frags = TigressDataToFragment(reinterpret_cast<uint32_t*>(ptr), banksize, event);
         } else if((banksize = event->LocateBank(nullptr, "GRF1", &ptr)) > 0) {
            frags = ProcessGriffin(reinterpret_cast<uint32_t*>(ptr), banksize, TGRSIDataParser::EBank::kGRF1, event);
         } else if((banksize = event->LocateBank(nullptr, "GRF2", &ptr)) > 0) {
            frags = ProcessGriffin(reinterpret_cast<uint32_t*>(ptr), banksize, TGRSIDataParser::EBank::kGRF2, event);
         } else if((banksize = event->LocateBank(nullptr, "GRF3", &ptr)) > 0) {
            frags = ProcessGriffin(reinterpret_cast<uint32_t*>(ptr), banksize, TGRSIDataParser::EBank::kGRF3, event);
         } else if((banksize = event->LocateBank(nullptr, "GRF4", &ptr)) > 0) {
            frags = ProcessGriffin(reinterpret_cast<uint32_t*>(ptr), banksize, TGRSIDataParser::EBank::kGRF4, event);
         } else if((banksize = event->LocateBank(nullptr, "CAEN", &ptr)) > 0) {
            frags = CaenPsdToFragment(reinterpret_cast<uint32_t*>(ptr), banksize, event);
         } else if((banksize = event->LocateBank(nullptr, "CPHA", &ptr)) > 0) {
            frags = CaenPhaToFragment(reinterpret_cast<uint32_t*>(ptr), banksize, event);
         } else if((banksize = event->LocateBank(nullptr, "MADC", &ptr)) > 0) {
            frags = EmmaMadcDataToFragment(reinterpret_cast<uint32_t*>(ptr), banksize, event);
				if((banksize = event->LocateBank(nullptr, "EMMT", &ptr)) > 0) {
					//TODO: make this smarter so that an error in processing EMMT bank doesn't reduce frags
					frags += EmmaTdcDataToFragment(reinterpret_cast<uint32_t*>(ptr), banksize, event);
				}
			} else if((banksize = event->LocateBank(nullptr, "EMMT", &ptr)) > 0) {
				frags = EmmaTdcDataToFragment(reinterpret_cast<uint32_t*>(ptr), banksize, event);
			} else {
				std::cout<<DRED<<std::endl<<"Unknown bank in midas event #"<<event->GetSerialNumber()<<", event ID 1, bank list "<<event->GetBankList()<<RESET_COLOR<<std::endl;
			}
			break;
		case 2:
			event->SetBankList();
         if((banksize = event->LocateBank(nullptr, "SRAW", &ptr)) > 0) {
				if(!TGRSIOptions::Get()->SuppressErrors()) {
					std::cout<<"Found bank \"SRAW\" of size "<<banksize<<std::endl;
				}
            frags = EmmaRawDataToFragment(reinterpret_cast<uint32_t*>(ptr), banksize, event);
			}
			if((banksize = event->LocateBank(nullptr, "SSUM", &ptr)) > 0) {
				if(!TGRSIOptions::Get()->SuppressErrors()) {
					std::cout<<"Found bank \"SSUM\" of size "<<banksize<<std::endl;
				}
            frags = EmmaSumDataToFragment(reinterpret_cast<uint32_t*>(ptr), banksize, event);
			} 
			if((banksize = event->LocateBank(nullptr, "SCLR", &ptr)) > 0) {
				if(!TGRSIOptions::Get()->SuppressErrors()) {
					std::cout<<"Found bank \"SCLR\" of size "<<banksize<<std::endl;
				}
			}
			break;
		case 3:
			if((banksize = event->LocateBank(nullptr, "CAEN", &ptr)) > 0) {
				frags = CaenPsdToFragment(reinterpret_cast<uint32_t*>(ptr), banksize, event);
			} else if((banksize = event->LocateBank(nullptr, "CPHA", &ptr)) > 0) {
				frags = CaenPhaToFragment(reinterpret_cast<uint32_t*>(ptr), banksize, event);
			} else {
				std::cout<<DRED<<std::endl<<"Unknown bank in midas event #"<<event->GetSerialNumber()<<", event ID 3, bank list "<<event->GetBankList()<<RESET_COLOR<<std::endl;
			}
			break;
		case 4:
		case 5:
			event->SetBankList();
			if((banksize = event->LocateBank(nullptr, "MSRD", &ptr)) > 0) {
				EPIXToScalar(reinterpret_cast<float*>(ptr), banksize, event->GetSerialNumber(), event->GetTimeStamp());
				// EPIXToScalar will only ever read a single fragment
				event->IncrementGoodFrags();
			}
			break;
		case 0x8001:
			// end of file ODB
#ifdef HAS_XML
			TXMLOdb*  odb     = new TXMLOdb(event->GetData(), event->GetDataSize());
			TXMLNode* node    = odb->FindPath("/Runinfo/Stop time binary");
			if(node != nullptr) {
				std::stringstream str(node->GetText());
				unsigned int odbTime;
				str>>odbTime;
				odbTime *= 10; // convert from 10 ns to 1 ns units
				if(atoi(node->GetText()) != 0 && odbTime != event->GetTimeStamp() && !TGRSIOptions::Get()->SuppressErrors()) {
					std::cout<<"Warning, ODB stop time of last subrun ("<<odbTime<<") does not match midas time of last event in this subrun ("<<event->GetTimeStamp()<<")!"<<std::endl;
				}
				TRunInfo::SetRunStop(event->GetTimeStamp());
			}
			TRunInfo::SetRunLength();
			delete odb;
#endif
			break;
		};
	} catch(const std::bad_alloc&) {
	}

	// if we failed to get any fragments and this is not a start-of-run or end-of-run event
	// we print an error message (unless these are suppressed)
	if(frags <= 0 && event->GetEventId() < 0x8000 && !TGRSIOptions::Get()->SuppressErrors()) {
		event->SetBankList();
		event->Print(Form("a%i", (-1 * frags) - 1));
	}

	if(frags < 0) {
		frags = 0;
	}

	return frags;
}

int TGRSIDataParser::TigressDataToFragment(uint32_t* data, int size, std::shared_ptr<TMidasEvent>& event)
{
	/// Converts A MIDAS File from the Tigress DAQ into a TFragment.
	int                        NumFragsFound = 0;
	std::shared_ptr<TFragment> eventFrag     = std::make_shared<TFragment>();
	eventFrag->SetDaqTimeStamp(event->GetTimeStamp());
	eventFrag->SetDaqId(event->GetSerialNumber());

	int      x     = 0;
	uint32_t dword = *(data + x);

	uint32_t type;
	uint32_t value;

	if(!SetTIGTriggerID(dword, eventFrag)) {
		std::cout<<RED<<"Setting TriggerId ("<<hex(dword, 8)<<") failed on midas event: "<<DYELLOW<<event->GetSerialNumber()<<RESET_COLOR<<std::endl;
		return -x;
	}
	x += 1;

	// There can be a tigger bit pattern between the header and the time !   pcb.

	if(!SetTIGTimeStamp((data + x), eventFrag)) {
		std::cout<<RED<<x<<" Setting TimeStamp failed on midas event: "<<DYELLOW<<event->GetSerialNumber()<<RESET_COLOR<<std::endl;
		return -x;
	}
	// int temp_charge =  0;
	int temp_led = 0;
	for(; x < size; x++) {
		dword = *(data + x);
		type  = (dword & 0xf0000000) >> 28;
		value = (dword & 0x0fffffff);
		switch(type) {
			case 0x0: // raw wave forms.
				{
					TChannel* chan = TChannel::GetChannel(eventFrag->GetAddress(), false);
					if(!NoWaveforms()) {
						SetTIGWave(value, eventFrag);
					}
					if((chan != nullptr) && strncmp("Tr", chan->GetName(), 2) == 0) {
						SetTIGWave(value, eventFrag);
					} else if((chan != nullptr) && strncmp("RF", chan->GetName(), 2) == 0) {
						SetTIGWave(value, eventFrag);
					}
				} break;
			case 0x1: // trapizodal wave forms.
				break;
			case 0x4: // cfd values.  This also ends the the fragment!
				SetTIGCfd(value, eventFrag);
				SetTIGLed(temp_led, eventFrag);
				/// check whether the fragment is 'good'

				if(((*(data + x + 1)) & 0xf0000000) != 0xe0000000) {
					std::shared_ptr<TFragment> transferfrag = std::make_shared<TFragment>(*eventFrag);
					eventFrag                               = std::make_shared<TFragment>();
					eventFrag->SetDaqTimeStamp(transferfrag->GetDaqTimeStamp());
					eventFrag->SetDaqId(transferfrag->GetDaqId());
					eventFrag->SetTriggerId(transferfrag->GetTriggerId());
					eventFrag->SetTimeStamp(transferfrag->GetTimeStamp());

					Push(GoodOutputQueues(), transferfrag);
					NumFragsFound++;
					event->IncrementGoodFrags();
				} else {
					std::shared_ptr<TFragment> transferfrag = std::make_shared<TFragment>(*eventFrag);
					Push(GoodOutputQueues(), transferfrag);
					NumFragsFound++;
					event->IncrementGoodFrags();
					eventFrag = nullptr;
					return NumFragsFound;
				}

				break;
			case 0x5: // raw charge evaluation.
				SetTIGCharge(value, eventFrag);
				break;
			case 0x6: temp_led = value; break;
			case 0xb:
						 // SetTIGBitPattern
						 break;
			case 0xc: SetTIGAddress(value, eventFrag); break;
			case 0xe: // this ends the bank!
						 if(eventFrag) {
							 return -x;
						 }
						 break;
			case 0xf: break;
			default: break;
		}
	}
	return NumFragsFound;
}

void TGRSIDataParser::SetTIGAddress(uint32_t value, const std::shared_ptr<TFragment>& currentFrag)
{
	/// Sets the digitizer address of the 'currentFrag' TFragment
	currentFrag->SetAddress(static_cast<int32_t>(0x00ffffff & value));
}

void TGRSIDataParser::SetTIGWave(uint32_t value, const std::shared_ptr<TFragment>& currentFrag)
{
	/// Sets the waveform for a Tigress event.

	if(currentFrag->GetWaveform()->size() > (100000)) {
		std::cout<<"number of wave samples found is to great"<<std::endl;
		return;
	}

	if((value & 0x00002000) != 0u) {
		int temp = value & 0x00003fff;
		temp     = ~temp;
		temp     = (temp & 0x00001fff) + 1;
		currentFrag->AddWaveformSample(static_cast<Short_t>(-temp));
	} else {
		currentFrag->AddWaveformSample(static_cast<Short_t>(value & 0x00001fff));
	}
	if(((value >> 14) & 0x00002000) != 0u) {
		int temp = (value >> 14) & 0x00003fff;
		temp     = ~temp;
		temp     = (temp & 0x00001fff) + 1;
		currentFrag->AddWaveformSample(static_cast<Short_t>(-temp));
	} else {
		currentFrag->AddWaveformSample(static_cast<Short_t>((value >> 14) & 0x00001fff));
	}
}

void TGRSIDataParser::SetTIGCfd(uint32_t value, const std::shared_ptr<TFragment>& currentFrag)
{
	/// Sets the CFD of a Tigress Event.

	currentFrag->SetCfd(int32_t(value & 0x07ffffff));
}

void TGRSIDataParser::SetTIGLed(uint32_t, const std::shared_ptr<TFragment>&)
{
	/// Sets the LED of a Tigress event.
	// No longer used anywhere
	//  currentFrag->SetLed( int32_t(value & 0x07ffffff) );
}

void TGRSIDataParser::SetTIGCharge(uint32_t value, const std::shared_ptr<TFragment>& currentFragment)
{
	/// Sets the integrated charge of a Tigress event.
	TChannel* chan = currentFragment->GetChannel();
	if(chan == nullptr) {
		chan = Channel();
	}
	std::string dig_type = chan->GetDigitizerTypeString();

	int charge;
	if((dig_type.compare(0, 5, "Tig10") == 0) || (dig_type.compare(0, 5, "TIG10") == 0)) {
		if((value & 0x02000000) != 0u) {
			charge = (-((~(static_cast<int32_t>(value) & 0x01ffffff)) & 0x01ffffff) + 1);
		} else {
			charge = (value & 0x03ffffff);
		}
	} else if((dig_type.compare(0, 5, "Tig64") == 0) || (dig_type.compare(0, 5, "TIG64") == 0)) {
		if((value & 0x00200000) != 0u) {
			charge = (-((~(static_cast<int32_t>(value) & 0x001fffff)) & 0x001fffff) + 1);
		} else {
			charge = ((value & 0x003fffff));
		}
	} else {
		if((value & 0x02000000) != 0u) {
			charge = (-((~(static_cast<int32_t>(value) & 0x01ffffff)) & 0x01ffffff) + 1);
		} else {
			charge = ((static_cast<int32_t>(value) & 0x03ffffff));
		}
	}
	currentFragment->SetCharge(charge);
}

bool TGRSIDataParser::SetTIGTriggerID(uint32_t value, const std::shared_ptr<TFragment>& currentFrag)
{
	/// Sets the Trigger ID of a Tigress event.
	if((value & 0xf0000000) != 0x80000000) {
		return false;
	}
	value                            = value & 0x0fffffff;
	unsigned int LastTriggerIdHiBits = LastTriggerId() & 0xFF000000; // highest 8 bits, remainder will be
	unsigned int LastTriggerIdLoBits = LastTriggerId() & 0x00FFFFFF; // determined by the reported value
	if(value < MaxTriggerId() / 10) {                                // the trigger id has wrapped around
		if(LastTriggerIdLoBits > MaxTriggerId() * 9 / 10) {
			currentFrag->SetTriggerId((uint64_t)(LastTriggerIdHiBits + value + MaxTriggerId()));
			std::cout<<DBLUE<<"We are looping new trigger id = "<<currentFrag->GetTriggerId()<<", last trigger hi bits = "<<LastTriggerIdHiBits<<", last trigger lo bits = "<<LastTriggerIdLoBits<<", value = "<<value<<RESET_COLOR<<std::endl;
		} else {
			currentFrag->SetTriggerId(static_cast<uint64_t>(LastTriggerIdHiBits + value));
		}
	} else if(value < MaxTriggerId() * 9 / 10) {
		currentFrag->SetTriggerId(static_cast<uint64_t>(LastTriggerIdHiBits + value));
	} else {
		if(LastTriggerIdLoBits < MaxTriggerId() / 10) {
			currentFrag->SetTriggerId((uint64_t)(LastTriggerIdHiBits + value - MaxTriggerId()));
			std::cout<<DRED<<"We are backwards looping new trigger id = "<<currentFrag->GetTriggerId()<<", last trigger hi bits = "<<LastTriggerIdHiBits<<", last trigger lo bits = "<<LastTriggerIdLoBits<<", value = "<<value<<RESET_COLOR<<std::endl;
		} else {
			currentFrag->SetTriggerId(static_cast<uint64_t>(LastTriggerIdHiBits + value));
		}
	}
	// fragment_id_map[value]++;
	// currentFrag->FragmentId = fragment_id_map[value];
	LastTriggerId(static_cast<unsigned long>(currentFrag->GetTriggerId()));
	return true;
}

bool TGRSIDataParser::SetTIGTimeStamp(uint32_t* data, const std::shared_ptr<TFragment>& currentFrag)
{
	/// Sets the Timestamp of a Tigress Event
	for(int x = 0; x < 10; x++) { // finds the timestamp.
		data = data + 1;
		if(((*data) >> 28) == 0xa) {
			break;
		}
	}
	long timestamplow  = -1;
	long timestamphigh = -1;


	if(!((*data & 0xf0000000) == 0xa0000000)) {
		std::cout<<"here 0?\t"<<hex(*data,8)<<std::endl;
		return false;
	}

	std::array<unsigned int, 5> time = {0}; // tigress can report up to 5 valid timestamp words
	int          x       = 0;

	while((*(data + x) & 0xf0000000) == 0xa0000000) {
		time[x] = *(data + x);
		x += 1;
		if(x == 5) {
			break;
		}
	}

	switch(x) {
		case 1: // bad.
			break;
		case 2:                     // minimum number of good a's
			if(time[0] != time[1]) { // tig64's only have two, both second hex's are 0s. also some times tig10s.
				timestamplow  = time[0] & 0x00ffffff;
				timestamphigh = time[1] & 0x00ffffff;
				// return true;
			}
			break;
		case 3:
			if(time[0] == time[1] && time[0] != time[2]) {
				if(((time[0] & 0x0f000000) != 0x00000000) && ((time[2] & 0x0f000000) != 0x01000000)) {
					break;
				}
				timestamplow  = time[0] & 0x00ffffff;
				timestamphigh = time[2] & 0x00ffffff;
			} else if(time[0] != time[1] && time[1] == time[2]) {
				if(((time[0] & 0x0f000000) != 0x00000000) && ((time[1] & 0x0f000000) != 0x01000000)) {
					break;
				}
				timestamplow  = time[0] & 0x00ffffff;
				timestamphigh = time[1] & 0x00ffffff;
			} else { // assume the third if the counter.
				// if( ((time[0]&0x0f000000)!=0x00000000) && ((time[1]&0x0f000000)!=0x01000000) )
				//   break;
				timestamplow  = time[0] & 0x00ffffff;
				timestamphigh = time[1] & 0x00ffffff;
			}
			break;
			// return true;
		case 4:
		case 5:
			if(time[0] == time[1] && time[2] == time[3]) {
				if(((time[0] & 0x0f000000) != 0x00000000) && ((time[2] & 0x0f000000) != 0x01000000)) {
					break;
				}
				timestamplow  = time[0] & 0x00ffffff;
				timestamphigh = time[1] & 0x00ffffff;
			} else {
				if(((time[0] & 0x0f000000) != 0x00000000) && ((time[1] & 0x0f000000) != 0x01000000)) {
					break;
				}
				timestamplow  = time[0] & 0x00ffffff;
				timestamphigh = time[1] & 0x00ffffff;
			}
			break;
			// return true;
	};
	if(timestamplow > -1 && timestamphigh > -1) {
		// combine low and high time stamp bits
		currentFrag->SetTimeStamp((timestamphigh<<24) + timestamplow);
		return true;
	}

	return false;
}

/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////

int TGRSIDataParser::ProcessGriffin(uint32_t* data, const int& size, const EBank& bank, std::shared_ptr<TMidasEvent>& event)
{
	// loop over words in event to find fragment header
	int totalFrags = 0;
	for(int index = 0; index < size;) {
		if(((data[index]) & 0xf0000000) == 0x80000000) {
			// if we found a fragment header we use GriffinDataToFragment which returns the number of words read
			int words;
			try {
				words = GriffinDataToFragment(&data[index], size - index, bank, event->GetSerialNumber(), event->GetTimeStamp());
			} catch(TGRSIDataParserException& e) {
				words = -e.GetFailedWord();
				if(!TGRSIOptions::Get()->SuppressErrors()) {
					if(!TGRSIOptions::Get()->LogErrors()) {
						std::cout<<std::endl<<e.what();
					}
				}
			}
			if(words > 0) {
				// we successfully read one event with <words> words, so we advance the index by words
				event->IncrementGoodFrags();
				++totalFrags;
				index += words;
			} else {
				// we failed to read the fragment on word <-words>, so advance the index by -words and we create an error
				// message
				++totalFrags; // if the midas bank fails, we assume it only had one frag in it... this is just used for a
				// print statement.
				index -= words;

				if(!TGRSIOptions::Get()->SuppressErrors()) {
					if(!TGRSIOptions::Get()->LogErrors()) {
						std::cout<<DRED<<"//**********************************************//"<<RESET_COLOR<<std::endl;
						std::cout<<DRED<<"Bad things are happening. Failed on datum "<<index<<RESET_COLOR<<std::endl;
						event->Print(Form("a%i", index));
						std::cout<<DRED<<"//**********************************************//"<<RESET_COLOR<<std::endl;
					} else {
						std::string errfilename = "error.log";
						// if(mFile) {
						//   if(mFile->GetSubRunNumber() != -1) {
						//     errfilename.append(Form("error%05i_%03i.log",mFile->GetRunNumber(),mFile->GetSubRunNumber()));
						//   } else {
						//     errfilename.append(Form("error%05i.log",mFile->GetRunNumber()));
						//   }
						// } else {
						//   errfilename.append("error_log.log");
						// }
						FILE* originalstdout = stdout;
						FILE* errfileptr     = freopen(errfilename.c_str(), "a", stdout);
						std::cout<<"//**********************************************//"<<std::endl;
						event->Print("a");
						std::cout<<"//**********************************************//"<<std::endl;
						fclose(errfileptr);
						stdout = originalstdout;
					}
				}
			}
		} else {
			// this is not a fragment header, so we advance the index
			++index;
		}
	}

	return totalFrags;
}

int TGRSIDataParser::GriffinDataToFragment(uint32_t* data, int size, EBank bank, unsigned int midasSerialNumber,
		time_t midasTime)
{
	/// Converts a Griffin flavoured MIDAS file into a TFragment and returns the number of words processed (or the
	/// negative index of the word it failed on)
	std::shared_ptr<TFragment> eventFrag = std::make_shared<TFragment>();
	// no need to delete eventFrag, it's a shared_ptr and gets deleted when it goes out of scope
	FragmentHasWaveform(false);
	fState               = EDataParserState::kGood;
	int failedWord = -1; // Variable stores which word we failed on (if we fail). This is somewhat duplicate information
	// to fState, but makes things easier to track.
	bool multipleErrors = false; // Variable to store if multiple errors occured parsing one fragment

	if(Options() == nullptr) {
		Options(TGRSIOptions::Get());
	}

	int           x   = 0;
	if(!SetGRIFHeader(data[x++], eventFrag, bank)) {
		std::cout<<DYELLOW<<"data[0] = "<<hex(data,8)<<RESET_COLOR<<std::endl;
		// we failed to get a good header, so we don't know which detector type this fragment would've belonged to
		TParsingDiagnostics::Get()->BadFragment(-1); 
		// this is the first word, so no need to check is the state/failed word has been set before
		fState     = EDataParserState::kBadHeader;
		failedWord = 0;
	}

	eventFrag->SetDaqTimeStamp(midasTime);
	eventFrag->SetDaqId(midasSerialNumber);

	// Changed on 11 Aug 2015 by RD to include PPG events. If the event has ModuleType 4 and address 0xffff, it is a PPG
	// event.
	if((eventFrag->GetModuleType() == 1 || eventFrag->GetModuleType() == 4) && eventFrag->GetAddress() == 0xffff) {
		return GriffinDataToPPGEvent(data, size, midasSerialNumber, midasTime);
	}
	// If the event has detector type 15 (0xf) it's a scaler event.
	if(eventFrag->GetDetectorType() == 0xf) {
		// a scaler event (trigger or deadtime) has 8 words (including header and trailer), make sure we have at least
		// that much left
		TChannel* chan = TChannel::GetChannel(eventFrag->GetAddress(), false);
		if((chan != nullptr) && strncmp("RF", chan->GetName(), 2) == 0){
			//JW: RF event, stored as a scaler containing fit paramters 
			//in recent (2019 and later) GRIF-16 firmware revisions.
			//These should be still be treated as RF event fragments, but 
			//need to be parsed differently.
			//std::cout << "RF scaler event" << std::endl;
			return RFScalerToFragment(data, size, eventFrag);
		}
		if(size < 8) {
			if(fState == EDataParserState::kGood) {
				fState     = EDataParserState::kMissingWords;
				failedWord = x;
			} else {
				multipleErrors = true;
			}
			throw TGRSIDataParserException(fState, failedWord, multipleErrors);
		}
		return GriffinDataToScalerEvent(data, eventFrag->GetAddress());
	}

	// If the flag is set, skip any fragment without a channel
	if(fIgnoreMissingChannel && eventFrag->GetChannel() != nullptr) {
		// find end of this event
		for(; x < size; x++) {
			if((data[x] >> 28) == 0xe) return x;
		}
	}

	// The Network packet number is for debugging and is not always written to the midas file.
	if(SetGRIFNetworkPacket(data[x], eventFrag)) {
		++x;
	}

	// The Primary Filter Pattern is in an unstable state right now and is not
	// always written to the midas file
	if(SetGRIFPrimaryFilterPattern(data[x], eventFrag, bank)) {
		++x;
	}

	// We can get multiple filter ids (one fragment can pass multiple filter conditions)
	// so we have to loop until we don't find one
	while(SetGRIFPrimaryFilterId(data[x], eventFrag)) {
		++x;
	}

	// The channel trigger ID is in an unstable state right now and is not
	// always written to the midas file
	if(!SetGRIFChannelTriggerId(data[x++], eventFrag)) {
		TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
		if(fState == EDataParserState::kGood) {
			fState     = EDataParserState::kBadTriggerId;
			failedWord = x - 1; // -1 compensates the incrementation in the if-statement
		} else {
			multipleErrors = true;
		}
	}

	// The Network packet number is for debugging and is not always written to
	// the midas file.
	if(SetGRIFNetworkPacket(data[x], eventFrag)) {
		x++;
	}

	if(!SetGRIFTimeStampLow(data[x++], eventFrag)) {
		TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
		if(fState == EDataParserState::kGood) {
			fState     = EDataParserState::kBadLowTS;
			failedWord = x - 1; // -1 compensates the incrementation in the if-statement
		} else {
			multipleErrors = true;
		}
	}

	if(!SetGRIFDeadTime(data[x++], eventFrag)) {
		TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
		if(fState == EDataParserState::kGood) {
			fState     = EDataParserState::kBadHighTS;
			failedWord = x - 1; // -1 compensates the incrementation in the if-statement
		} else {
			multipleErrors = true;
		}
	}

	std::vector<Int_t>   tmpCharge;
	std::vector<Short_t> tmpIntLength;
	std::vector<Int_t>   tmpCfd;

	for(; x < size; x++) {
		uint32_t dword  = data[x];
		uint32_t packet = dword >> 28;
		uint32_t value  = dword & 0x0fffffff;

		switch(packet) {
			case 0x8: // The 8 packet type is for event headers
				// if this happens, we have "accidentally" found another event.
				// currently the GRIF-C only sets the primary/secondary port of the address for the first header (of the corrupt
				// event)
				// so we want to ignore this corrupt event and the next event which has a wrong address
				TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
				if(fState == EDataParserState::kGood) {
					fState     = EDataParserState::kSecondHeader;
					failedWord = x + 1; //+1 to ensure we don't read this header as start of a good event
				} else {
					multipleErrors = true;
				}
				Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
				throw TGRSIDataParserException(fState, failedWord, multipleErrors);
				break;
			case 0xc: // The c packet type is for waveforms
				if(!NoWaveforms()) {
					SetGRIFWaveForm(value, eventFrag);
				}
				break;
			case 0xd:
				SetGRIFNetworkPacket(dword, eventFrag); // The network packet placement is not yet stable.
				break;
			case 0xe:
				// changed on 21 Apr 2015 by JKS, when signal processing code from Chris changed the trailer.
				// change should be backward-compatible
				if((value & 0x3fff) == (eventFrag->GetChannelId() & 0x3fff)) {
					if(!Options()->SuppressErrors() && (eventFrag->GetModuleType() == 2) && (bank < EBank::kGRF3)) {
						// check whether the nios finished and if so whether it finished with an error
						if(((value >> 14) & 0x1) == 0x1) {
							if(((value >> 16) & 0xff) != 0) {
								std::cout<<BLUE<<hex(eventFrag->GetAddress(),4)<<": NIOS code finished with error "<<hex((value >> 16) & 0xff,2)<<RESET_COLOR<<std::endl;
							}
						}
					}

					if((eventFrag->GetModuleType() == 1) || (bank > EBank::kGRF2)) { // 4Gs have this only for banks newer than GRF2
						eventFrag->SetAcceptedChannelId((value >> 14) & 0x3fff);
					} else {
						eventFrag->SetAcceptedChannelId(0);
					}

					// check the number of words the header said we should have with the number of words we've read
					// the number of words is only set for bank >= GRF3
					// if the fragment has a waveform, we can't compare the number of words
					// the headers number of words includes the header (word 0) itself, so we need to compare to x plus one
					if(Options()->WordOffset() >= 0 && eventFrag->GetNumberOfWords() > 0 && !eventFrag->HasWave() && eventFrag->GetNumberOfWords() != x + Options()->WordOffset()) {
						if(fState == EDataParserState::kGood) {
							fState     = EDataParserState::kWrongNofWords;
							failedWord = x;
						} else {
							multipleErrors = true;
						}
						Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
						throw TGRSIDataParserException(fState, failedWord, multipleErrors);
					}

					// the way we insert the fragment(s) depends on the module type and bank:
					// 1 - for module type 1 & bank GRF4, we can't insert the fragments yet, we need to put them in a separate queue
					// 2 - for module type 2 (4G, all banks) and module type 1 & bank GRF3 we set the single charge, cfd, and
					//     IntLength, and insert the fragment
					// 3 - for module type 1 & banks GRF1/GRF2 we loop over the charge, cfd, and IntLengths, and insert the
					//     (multiple) fragment(s)
					// the last two cases can be treated the same since the second case will just have a single length charge,
					// cfd, and IntLengths

					// so we only need to check for the first case
					if(eventFrag->GetModuleType() == 1 && bank == EBank::kGRF4) {
						if(tmpCfd.size() != 1) {
							if(RecordDiag()) {
								TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
							}
							if(fState == EDataParserState::kGood) {
								fState     = EDataParserState::kNotSingleCfd;
								failedWord = x;
							} else {
								multipleErrors = true;
							}
							Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
							throw TGRSIDataParserException(fState, failedWord, multipleErrors);
						}
						eventFrag->SetCfd(tmpCfd[0]);
						if(RecordDiag()) {
							TParsingDiagnostics::Get()->GoodFragment(eventFrag->GetDetectorType());
						}
						FragmentMap().Add(eventFrag, tmpCharge, tmpIntLength);
						return x;
					}
					if(tmpCharge.size() != tmpIntLength.size() || tmpCharge.size() != tmpCfd.size()) {
						if(RecordDiag()) {
							TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
						}
						if(fState == EDataParserState::kGood) {
							fState     = EDataParserState::kSizeMismatch;
							failedWord = x;
						} else {
							multipleErrors = true;
						}
						Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
						throw TGRSIDataParserException(fState, failedWord, multipleErrors);
					}
					for(size_t h = 0; h < tmpCharge.size(); ++h) {
						eventFrag->SetCharge(tmpCharge[h]);
						eventFrag->SetKValue(tmpIntLength[h]);
						eventFrag->SetCfd(tmpCfd[h]);
						if(RecordDiag()) {
							TParsingDiagnostics::Get()->GoodFragment(eventFrag->GetDetectorType());
						}
						if(fState == EDataParserState::kGood) {
							if(Options()->ReconstructTimeStamp()) {
								LastTimeStampMap()[eventFrag->GetAddress()] = eventFrag->GetTimeStamp();
							}
							Push(GoodOutputQueues(), std::make_shared<TFragment>(*eventFrag));
						} else {
							if(Options()->ReconstructTimeStamp() && fState == EDataParserState::kBadHighTS && !multipleErrors) {
								// std::cout<<"reconstructing timestamp from 0x"<<std::hex<<eventFrag->GetTimeStamp()<<" using
								// 0x"<<LastTimeStampMap()[eventFrag->GetAddress()];
								// reconstruct the high bits of the timestamp from the high bits of the last time stamp of the
								// same address after converting the saved timestamp back to 10 ns units
								if((eventFrag->GetTimeStamp() & 0x0fffffff) <
										(LastTimeStampMap()[eventFrag->GetAddress()] & 0x0fffffff)) {
									// we had a wrap-around of the low time stamp, so we need to set the high bits to the old
									// high bits plus one
									eventFrag->AppendTimeStamp(((LastTimeStampMap()[eventFrag->GetAddress()] >> 28) + 1)<<28);
								} else {
									eventFrag->AppendTimeStamp(LastTimeStampMap()[eventFrag->GetAddress()] & 0x3fff0000000);
								}
								// std::cout<<" => 0x"<<eventFrag->GetTimeStamp()<<std::dec<<std::endl;
								Push(GoodOutputQueues(), std::make_shared<TFragment>(*eventFrag));
							} else {
								// std::cout<<"Can't reconstruct time stamp, "<<Options()->ReconstructTimeStamp()<<",
								// state "<<fState<<" = "<<EDataParserState::kBadHighTS<<", "<<multipleErrors<<std::endl;
								Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
							}
						}
					}
					return x;
				} else {
					if(RecordDiag()) {
						TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
					}
					if(fState == EDataParserState::kGood) {
						fState     = EDataParserState::kBadFooter;
						failedWord = x;
					} else {
						multipleErrors = true;
					}
					Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
					throw TGRSIDataParserException(fState, failedWord, multipleErrors);
				}
				break;
			case 0xf:
				switch(bank) {
					case EBank::kGRF1: // format from before May 2015 experiments
						TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
						if(fState == EDataParserState::kGood) {
							fState     = EDataParserState::kFault;
							failedWord = x;
						} else {
							multipleErrors = true;
						}
						Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
						throw TGRSIDataParserException(fState, failedWord, multipleErrors);
						break;
					case EBank::kGRF2: // from May 2015 to the end of 2015 0xf denoted a psd-word from a 4G
						if(x + 1 < size) {
							SetGRIFCc(value, eventFrag);
							++x;
							dword = data[x];
							SetGRIFPsd(dword, eventFrag);
						} else {
							TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
							if(fState == EDataParserState::kGood) {
								fState     = EDataParserState::kMissingPsd;
								failedWord = x;
							} else {
								multipleErrors = true;
							}
							Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
							throw TGRSIDataParserException(fState, failedWord, multipleErrors);
						}
						break;
					case EBank::kGRF3: // from 2016 on we're back to reserving 0xf for faults
					case EBank::kGRF4:
						TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
						if(fState == EDataParserState::kGood) {
							fState     = EDataParserState::kFault;
							failedWord = x;
						} else {
							multipleErrors = true;
						}
						Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
						throw TGRSIDataParserException(fState, failedWord, multipleErrors);
						break;
					default: 
						std::cout<<"This bank not yet defined."<<std::endl;
						break;
				}
				break;

			default:
				// these are charge/cfd words which are different depending on module type, and bank number/detector type
				switch(eventFrag->GetModuleType()) {
					case 1:
						switch(bank) { // the GRIF-16 data format depends on the bank number
							case EBank::kGRF1:    // bank's 1&2 have n*2 words with (5 high bits IntLength, 26 Charge)(5 low bits IntLength, 26
								// Cfd)
							case EBank::kGRF2:
								// read this pair of charge/cfd words, check if the next word is also a charge/cfd word
								if(((data[x] & 0x80000000) == 0x0) && x + 1 < size && (data[x + 1] & 0x80000000) == 0x0) {
									Short_t tmp = (data[x] & 0x7c000000) >> 21; // 21 = 26 minus space for 5 low bits
									tmpCharge.push_back((data[x] & 0x03ffffff) | (((data[x] & 0x02000000) == 0x02000000)
												? 0xfc000000
												: 0x0)); // extend the sign bit of 26bit charge word
									++x;
									tmpIntLength.push_back(tmp | ((data[x] & 0x7c000000) >> 26));
									tmpCfd.push_back(data[x] & 0x03ffffff);
								} else {
									TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
									if(fState == EDataParserState::kGood) {
										fState     = EDataParserState::kMissingCfd;
										failedWord = x;
									} else {
										multipleErrors = true;
									}
									Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
									throw TGRSIDataParserException(fState, failedWord, multipleErrors);
								}
								break;
							case EBank::kGRF3: // bank 3 has 2 words with (5 high bits IntLength, 26 Charge)(9 low bits IntLength, 22 Cfd)
								if(x + 1 < size &&
										(data[x + 1] & 0x80000000) == 0x0) {        // check if the next word is also a charge/cfd word
									Short_t tmp = (data[x] & 0x7c000000) >> 17; // 17 = 26 minus space for 9 low bits
									tmpCharge.push_back((data[x] & 0x03ffffff) | (((data[x] & 0x02000000) == 0x02000000)
												? 0xfc000000
												: 0x0)); // extend the sign bit of 26bit charge word
									++x;
									tmpIntLength.push_back(tmp | ((data[x] & 0x7fc00000) >> 22));
									tmpCfd.push_back(data[x] & 0x003fffff);
									break;
								} else {
									TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
									if(fState == EDataParserState::kGood) {
										fState     = EDataParserState::kMissingCfd;
										failedWord = x;
									} else {
										multipleErrors = true;
									}
									Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
									throw TGRSIDataParserException(fState, failedWord, multipleErrors);
								}
								break;
							case EBank::kGRF4: // bank 4 can have more than one integration (up to four), but these have to be combined with
								// other fragments/hits!
								// we always have 2 words with (5 high bits IntLength, 26 Charge)(9 low bits IntLength, 22 Cfd)
								if((data[x] & 0x80000000) == 0x0 && x + 1 < size &&
										(data[x + 1] & 0x80000000) == 0x0) { // check if the next word is also a charge/cfd word
									Short_t tmp = ((data[x] & 0x7c000000) >> 17) |
										(((data[x] & 0x40000000) == 0x40000000) ? 0xc000 : 0x0); // 17 = 26 minus space for 9
									// low bits; signed, so we
									// extend the sign bit from 14
									// (31) to 16 bits
                           tmpCharge.push_back((data[x] & 0x01ffffff) |
                                               (((data[x] & 0x01000000) == 0x01000000)
                                                   ? 0xfe000000
                                                   : 0x0));   // extend the sign bit of 25bit charge word
                           ++x;
                           tmpIntLength.push_back(tmp | ((data[x] & 0x7fc00000) >> 22));
									tmpCfd.push_back(data[x] & 0x003fffff);
									// check if we have two more words (X & XI) with (8 num hits, 2 reserved, 14 IntLength2)(31 Charge2);
									// x has already been incremented once!
									if(x + 2 < size && (data[x + 1] & 0x80000000) == 0x0 && (data[x + 2] & 0x80000000) == 0x0) {
										++x;
										tmpIntLength.push_back((data[x] & 0x3fff) | (((data[x] & 0x2000) == 0x2000) ? 0xc000 : 0x0));
										// eventFrag->SetNumberOfPileups((data[x] >> 16) & 0xff);
										++x;
										if((data[x] & 0x02000000) == 0x02000000) { // overflow bit was set
											tmpCharge.push_back(std::numeric_limits<int>::max());
										} else {
											tmpCharge.push_back((data[x] & 0x01ffffff) |
													(((data[x] & 0x01000000) == 0x01000000)
													 ? 0xfe000000
													 : 0x0)); // extend the sign bit of 25bit charge word
										}
										// check if we have two more words (XI & XIII) with (14 IntLength4, 2 reserved, 14 IntLength3)(31
										// Charge3); x has already been incremented thrice!
										if(x + 2 < size && (data[x + 1] & 0x80000000) == 0x0 && (data[x + 2] & 0x80000000) == 0x0) {
											++x;
											tmpIntLength.push_back((data[x] & 0x3fff) | (((data[x] & 0x2000) == 0x2000) ? 0xc000 : 0x0));
											tmpIntLength.push_back((data[x] >> 16) |
													(((data[x] & 0x20000000) == 0x20000000) ? 0xc000 : 0x0));
											++x;
											if((data[x] & 0x02000000) == 0x02000000) { // overflow bit was set
												tmpCharge.push_back(std::numeric_limits<int>::max());
											} else {
												tmpCharge.push_back((data[x] & 0x01ffffff) |
														(((data[x] & 0x01000000) == 0x01000000)
														 ? 0xfe000000
														 : 0x0)); // extend the sign bit of 25bit charge word
											}
											// check if we have one final word with (31 Charge4), otherwise remove the last integration
											// length (IntLength4)
											if(x + 1 < size && (data[x + 1] & 0x80000000) == 0x0) {
												++x;
												if((data[x] & 0x02000000) == 0x02000000) { // overflow bit was set
													tmpCharge.push_back(std::numeric_limits<int>::max());
												} else {
													tmpCharge.push_back((data[x] & 0x01ffffff) |
															(((data[x] & 0x01000000) == 0x01000000)
															 ? 0xfe000000
															 : 0x0)); // extend the sign bit of 25bit charge word
												}
											} else {
												tmpIntLength.pop_back();
											}
										} else if((data[x + 1] & 0x80000000) == 0x0) { // 5 words
											++x;
										}
									} else if((data[x + 1] & 0x80000000) == 0x0) { // 3 words
										++x;
									}
								} else {
									// these types of corrupt events quite often end without a trailer which leads to the header of the
									// next event missing the primary/secondary part of the address
									// so we look for the next trailer and stop there
									while(x < size && (data[x] & 0xf0000000) != 0xe0000000) {
										++x;
									}
									TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
									if(fState == EDataParserState::kGood) {
										fState     = EDataParserState::kMissingCharge;
										failedWord = x;
									} else {
										multipleErrors = true;
									}
									Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
									throw TGRSIDataParserException(fState, failedWord, multipleErrors);
								}
								break;
							default:
								if(!Options()->SuppressErrors()) {
									std::cout<<DRED<<"Error, bank type "<<static_cast<std::underlying_type<EBank>::type>(bank)<<" not implemented yet"<<RESET_COLOR<<std::endl;
								}
								TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
								if(fState == EDataParserState::kGood) {
									fState     = EDataParserState::kBadBank;
									failedWord = x;
								} else {
									multipleErrors = true;
								}
								Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
								throw TGRSIDataParserException(fState, failedWord, multipleErrors);
								break;
						}
						break;
					case 2:
						// the 4G data format depends on the detector type, but the first two words are always the same
						if(x + 1 < size && (data[x + 1] & 0x80000000) == 0x0) { // check if the next word is also a charge/cfd word
							Short_t tmp = (data[x] & 0x7c000000) >> 21;          // 21 = 26 minus space for 5 low bits
							tmpCharge.push_back((data[x] & 0x03ffffff) | (((data[x] & 0x02000000) == 0x02000000)
										? 0xfc000000
										: 0x0)); // extend the sign bit of 26bit charge word
							++x;
							tmpIntLength.push_back(tmp | ((data[x] & 0x7c000000) >> 26));
							tmpCfd.push_back(data[x] & 0x03ffffff);
						} else {
							TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
							if(fState == EDataParserState::kGood) {
								fState     = EDataParserState::kMissingCfd;
								failedWord = x;
							} else {
								multipleErrors = true;
							}
							Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
							throw TGRSIDataParserException(fState, failedWord, multipleErrors);
						}
						// for descant types (6,10,11) there are two more words for banks > GRF2 (bank GRF2 used 0xf packet and bank
						// GRF1 never had descant)
						if(bank > EBank::kGRF2 && (eventFrag->GetDetectorType() == 6 || eventFrag->GetDetectorType() == 10 ||
									eventFrag->GetDetectorType() == 11)) {
							++x;
							if(x + 1 < size && (data[x + 1] & 0x80000000) == 0x0) {
								SetGRIFCc(value, eventFrag);
								++x;
								dword = data[x];
								SetGRIFPsd(dword, eventFrag);
							} else {
								TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
								if(fState == EDataParserState::kGood) {
									fState     = EDataParserState::kMissingPsd;
									failedWord = x;
								} else {
									multipleErrors = true;
								}
								Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
								throw TGRSIDataParserException(fState, failedWord, multipleErrors);
							}
						}
						break;
					default:
						if(!Options()->SuppressErrors()) {
							std::cout<<DRED<<"Error, module type "<<eventFrag->GetModuleType()<<" not implemented yet"<<RESET_COLOR<<std::endl;
						}
						TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
						if(fState == EDataParserState::kGood) {
							fState     = EDataParserState::kBadModuleType;
							failedWord = x;
						} else {
							multipleErrors = true;
						}
						Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
						throw TGRSIDataParserException(fState, failedWord, multipleErrors);
				} // switch(eventFrag->GetModuleType())
				break;
		} // switch(packet)
	}    // for(;x<size;x++)

	TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
	if(fState == EDataParserState::kGood) {
		fState     = EDataParserState::kEndOfData;
		failedWord = x;
	} else {
		multipleErrors = true;
	}
	Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
	throw TGRSIDataParserException(fState, failedWord, multipleErrors);
	return -x;
}

bool TGRSIDataParser::SetGRIFHeader(uint32_t value, const std::shared_ptr<TFragment>& frag, EBank bank)
{
	if((value & 0xf0000000) != 0x80000000) {
		return false;
	}
	switch(bank) {
		case EBank::kGRF1: // header format from before May 2015 experiments
			// Sets:
			//     The number of filters
			//     The Data Type
			//     Number of Pileups
			//     Channel Address
			//     Detector Type
			// frag->SetNumberOfFilters((value &0x0f000000)>> 24);
			frag->SetModuleType((value & 0x00e00000) >> 21);
			frag->SetNumberOfPileups((value & 0x001c0000) >> 18);
			frag->SetAddress((value & 0x0003fff0) >> 4);
			frag->SetDetectorType((value & 0x0000000f));

			// if(frag-DetectorType==2)
			//    frag->ChannelAddress += 0x8000;
			break;
		case EBank::kGRF2:
			// Sets:
			//     The number of filters
			//     The Data Type
			//     Number of Pileups
			//     Channel Address
			//     Detector Type
			frag->SetNumberOfPileups((value & 0x0c000000) >> 26);
			frag->SetModuleType((value & 0x03800000) >> 23);
			// frag->SetNumberOfFilters((value &0x00700000)>> 20);
			frag->SetAddress((value & 0x000ffff0) >> 4);
			frag->SetDetectorType((value & 0x0000000f));

			break;
		case EBank::kGRF3:
		case EBank::kGRF4:
			frag->SetModuleType((value & 0x0e000000) >> 25);
			frag->SetNumberOfWords((value & 0x01f00000) >> 20);
			frag->SetAddress((value & 0x000ffff0) >> 4);
			frag->SetDetectorType((value & 0x0000000f));

			break;
		default: 
			std::cout<<"This bank is not yet defined."<<std::endl;
			return false;
	}

	return true;
}

bool TGRSIDataParser::SetGRIFPrimaryFilterPattern(uint32_t value, const std::shared_ptr<TFragment>& frag, EBank bank)
{
	/// Sets the Griffin Primary Filter Pattern
	if((value & 0xc0000000) != 0x00000000) {
		return false;
	}
	switch(bank) {
		case EBank::kGRF1:
		case EBank::kGRF2:
			frag->SetTriggerBitPattern(value >> 16);
			// frag->SetPPGWord(value & 0x0000ffff);//This is due to new GRIFFIN data format
			break;
		case EBank::kGRF3:
		case EBank::kGRF4:
			frag->SetTriggerBitPattern(value >> 16);
			frag->SetNumberOfPileups(value & 0x1f);
			FragmentHasWaveform((value & 0x8000) == 0x8000);
			break;
		default: return false;
	}
	return true;
}

bool TGRSIDataParser::SetGRIFPrimaryFilterId(uint32_t value, const std::shared_ptr<TFragment>& frag)
{
	/// Sets the Griffin Primary filter ID and PPG
	if((value & 0x80000000) != 0x00000000) {
		return false;
	}

	frag->SetTriggerId(value & 0x7FFFFFFF); // REAL
	return true;
}

bool TGRSIDataParser::SetGRIFChannelTriggerId(uint32_t value, const std::shared_ptr<TFragment>& frag)
{
	/// Sets the Griffin Channel Trigger ID
	if((value & 0xf0000000) != 0x90000000) {
		return false;
	}
	frag->SetChannelId(value & 0x0fffffff);
	return true;
}

bool TGRSIDataParser::SetGRIFNetworkPacket(uint32_t value, const std::shared_ptr<TFragment>& frag)
{
	/// Ignores the network packet number (for now)
	if((value & 0xf0000000) != 0xd0000000) {
		return false;
	}
	if((value & 0x0f000000) == 0x0f000000 && frag->GetNetworkPacketNumber() > 0) {
		if(frag->GetZc() != 0) {
			return false;
		}
		// descant zero crossing time.
		frag->SetZc(value & 0x00ffffff);
	} else {
		frag->SetNetworkPacketNumber(value & 0x0fffffff);
	}
	return true;
}

bool TGRSIDataParser::SetGRIFTimeStampLow(uint32_t value, const std::shared_ptr<TFragment>& frag)
{
	/// Sets the lower 28 bits of the griffin time stamp
	if((value & 0xf0000000) != 0xa0000000) {
		return false;
	}
	// we always get the lower 28 bits first
	frag->SetTimeStamp(value & 0x0fffffff);
	return true;
}

bool TGRSIDataParser::SetGRIFWaveForm(uint32_t value, const std::shared_ptr<TFragment>& frag)
{
	/// Sets the Griffin waveform if record_waveform is set to true
	if(frag->GetWaveform()->size() > 100000) {
		std::cout<<"number of wave samples found is too great"<<std::endl;
		return false;
	}

	// to go from a 14-bit signed number to a 16-bit signed number, we simply set the two highest bits if the sign bit is set
	frag->AddWaveformSample((value & 0x2000) != 0u ? static_cast<Short_t>((value & 0x3fff) | 0xc000)
			: static_cast<Short_t>(value & 0x3fff));
	value = value >> 14;
	frag->AddWaveformSample((value & 0x2000) != 0u ? static_cast<Short_t>((value & 0x3fff) | 0xc000)
			: static_cast<Short_t>(value & 0x3fff));

	return true;
}

bool TGRSIDataParser::SetGRIFDeadTime(uint32_t value, const std::shared_ptr<TFragment>& frag)
{
	/// Sets the Griffin deadtime and the upper 14 bits of the timestamp
	if((value & 0xf0000000) != 0xb0000000) {
		return false;
	}
	frag->SetDeadTime((value & 0x0fffc000) >> 14);
	// AppendTimeStamp simply adds the new value (not bitwise operation), so we just add the high bits
	frag->AppendTimeStamp(static_cast<Long64_t>(value & 0x00003fff)<<28);
	return true;
}

bool TGRSIDataParser::SetGRIFCc(uint32_t value, const std::shared_ptr<TFragment>& frag)
{
	/// set the short integration and the lower 9 bits of the long integration
	if(frag->GetCcShort() != 0 || frag->GetCcLong() != 0) {
		return false;
	}
	frag->SetCcShort(value & 0x7ffff);
	frag->SetCcLong(value >> 19);
	return true;
}

bool TGRSIDataParser::SetGRIFPsd(uint32_t value, const std::shared_ptr<TFragment>& frag)
{
	/// set the zero crossing and the higher 10 bits of the long integration
	if(frag->GetZc() != 0) { // low bits of ccLong have already been set
		return false;
	}
	frag->SetZc(value & 0x003fffff);
	frag->SetCcLong(frag->GetCcLong() | ((value & 0x7fe00000) >> 12)); // 21 bits from zero crossing minus 9 lower bits
	return true;
}

int TGRSIDataParser::RFScalerToFragment(uint32_t* data, const int size, const std::shared_ptr<TFragment>& frag)
{
	// Parses special RF scaler events which contain only timestamps and fit paramteters

   ULong64_t             ts = 0;
   ULong64_t             tshigh;
   double                rfFreq = -1.0;
   std::array<double, 4> rfPar;
   bool                  freqSet    = false;
   bool                  tsSet      = false;
   int                   failedWord = -1;

   int x=0;
	for(int i = 0; i < size; i++) {
		
		if(x >= size){
			//std::cout << "RF fragment missing frequency and/or timestamp info!" << std::endl;
			return -1;
		}

		uint32_t dword  = data[x];
		uint32_t packet = dword >> 28;
		uint32_t value  = dword & 0x0fffffff;

		switch(packet) {
			case 0x9:
				//RF frequency word
				rfFreq = static_cast<double>(value); //RF frequency (27-bit number) in cycles per 2^27 clock ticks or 1.34s
				freqSet=true;
				break;
			case 0xa:
				//timestamp words (low 0xa followed by high 0xb)
				ts = 0;
				ts |= value;
				x++;
				if((data[x] >> 28) == 0xb) {
					tshigh = data[x]; //need to convert the high timestamp data to a long int prior to shifting
					ts |= ((tshigh & 0x00003fff)<<28); //timestamp, in samples
					tsSet=true;
				}else{
					TParsingDiagnostics::Get()->BadFragment(frag->GetDetectorType());
					fState     = EDataParserState::kBadRFScalerWord;
					failedWord = x;
					Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*frag, data, size, failedWord, false));
					//std::cout << "Invalid RF high time stamp!" << std::endl;
					return -1;
				}
				break;
			case 0xe:
				//std::cout << "Early end to RF fragment." << std::endl;
				TParsingDiagnostics::Get()->BadFragment(frag->GetDetectorType());
				fState     = EDataParserState::kEndOfData;
				failedWord = x;
				Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*frag, data, size, failedWord, false));
				return -1;
				break;
			default:
				break;
		}
		x++;
		if(freqSet&&tsSet)
			break;
	}

	if(rfFreq < 0.0){
		//std::cout << "Bad RF frequency." << std::endl;
		TParsingDiagnostics::Get()->BadFragment(frag->GetDetectorType());
		fState     = EDataParserState::kUndefined;
		failedWord = x;
		Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*frag, data, size, failedWord, false));
		return -1;
	}

	frag->SetTimeStamp(ts);

	if(!(x<size-3)){
		//std::cout << "RF fragment does not contain all parameters." << std::endl;
		TParsingDiagnostics::Get()->BadFragment(frag->GetDetectorType());
		fState     = EDataParserState::kBadRFScalerWord;
		failedWord = x;
		Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*frag, data, size, failedWord, false));
		return -1;
	}
	if((data[x]==data[x+1])&&(data[x]==data[x+2])&&(data[x]==data[x+3])){
		//std::cout << "Failed RF fit: all parameters are the same value." << std::endl;
		TParsingDiagnostics::Get()->BadFragment(frag->GetDetectorType());
		fState     = EDataParserState::kBadRFScalerWord;
		failedWord = x;
		Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*frag, data, size, failedWord, false));
		return -1;
	}

	int pos=0;
	for(int i = 0; i < 4; i++) {

		uint32_t dword  = data[x];
		uint32_t packet = dword >> 28;

		if(x<size){
			if((packet) == 0xe){
				//std::cout << "RF fragment unexpectedly ended early!" << std::endl;
				TParsingDiagnostics::Get()->BadFragment(frag->GetDetectorType());
				fState     = EDataParserState::kEndOfData;
				failedWord = x;
				Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*frag, data, size, failedWord, false));
				return -1;
			}
			if((i!=2)&&(dword == 0)){
				//std::cout << "Failed RF fit: non-offset parameter is zero." << std::endl;
				TParsingDiagnostics::Get()->BadFragment(frag->GetDetectorType());
				fState     = EDataParserState::kBadRFScalerWord;
				failedWord = x;
				Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*frag, data, size, failedWord, false));
				return -1;
			}
			
			if(dword & (1<<29)){ 
				//parameter value is negative
				//take two's complement
				for(int j=0;j<30;j++){
					if(dword & (1<<j)){
						pos=j;
						break;
					}
				}
				for(int j=pos+1;j<30;j++){
					dword ^= 1 << j;
				}
				rfPar[i] = static_cast<double>(-1.0*(dword & 0x03ffffff)); //RF fit parameters (30-bit numbers)
			}else{
				rfPar[i] = static_cast<double>(dword & 0x03ffffff); //RF fit parameters (30-bit numbers)
			}
			
			x++;
		}
	}

	//RF frequency and all 4 fit parameters were parsed without issue
	//convert these into the RF period and phase
	
	rfPar[0] = rfPar[0] / rfPar[3];
	rfPar[1] = rfPar[1] / rfPar[3];
	rfPar[2] = rfPar[2] / rfPar[3];
	double T = 1.34217728E9 / rfFreq; // period in ns
	double A = sqrt(rfPar[1] * rfPar[1] + rfPar[0] * rfPar[0]);
	double s = -rfPar[0] / A;
	double c = rfPar[1] / A;
	double rfPhaseShift; //the phase shift, in ns
	if(s >= 0) {
		rfPhaseShift = acos(c) * T / (2 * TMath::Pi());
	} else {
		rfPhaseShift = (1 - acos(c) / (2 * TMath::Pi())) * T;
	}

	//put the important info (phase shift, period) into the fragment

	frag->SetCharge(static_cast<float>(T)); //period stored as charge (where else would I put it?)
	frag->SetCfd(static_cast<float>(rfPhaseShift) * 1.6f); //phase shift in cfd units (this one seems reasonable)

	Push(GoodOutputQueues(), std::make_shared<TFragment>(*frag));
	return 1;
}

int TGRSIDataParser::GriffinDataToPPGEvent(uint32_t* data, int size, unsigned int, time_t)
{
	auto* ppgEvent = new TPPGData;
	int   x        = 1; // We have already read the header so we can skip the 0th word.

	// The Network packet number is for debugging and is not always written to
	// the midas file.
	if(SetPPGNetworkPacket(data[x], ppgEvent)) { // The network packet placement is not yet stable.
		++x;
	}
	if(SetNewPPGPattern(data[x], ppgEvent)) {
		++x;
	}

	for(; x < size; x++) {
		uint32_t dword  = data[x];
		uint32_t packet = dword & 0xf0000000;
		uint32_t value  = dword & 0x0fffffff;

		switch(packet) {
			case 0x80000000: // The 8 packet type is for event headers
				// if this happens, we have "accidentally" found another event.
				return -x;
			case 0x90000000: // The b packet type contains the dead-time word
				SetOldPPGPattern(value, ppgEvent);
				break;
			case 0xd0000000:
				SetPPGNetworkPacket(dword, ppgEvent); // The network packet placement is not yet stable.
				break;
			case 0xa0000000: SetPPGLowTimeStamp(value, ppgEvent); break;
			case 0xb0000000: SetPPGHighTimeStamp(value, ppgEvent); break;
			case 0xe0000000:
								  // if((value & 0xFFFF) == (ppgEvent->GetNewPPG())){
								  TPPG::Get()->AddData(ppgEvent);
								  TParsingDiagnostics::Get()->GoodFragment(-2); // use detector type -2 for PPG
								  return x;
								  //} else  {
								  //	TParsingDiagnostics::Get()->BadFragment(-2); //use detector type -2 for PPG
								  //	return -x;
								  //}
								  break;
		};
	}
	delete ppgEvent;
	// No trailer found
	TParsingDiagnostics::Get()->BadFragment(-2); // use detector type -2 for PPG
	return -x;
}

bool TGRSIDataParser::SetNewPPGPattern(uint32_t value, TPPGData* ppgevent)
{
	if((value & 0xf0000000) != 0x00000000) {
		return false;
	}
	ppgevent->SetNewPPG(value & 0x0fffffff);
	return true;
}

bool TGRSIDataParser::SetOldPPGPattern(uint32_t value, TPPGData* ppgevent)
{
	ppgevent->SetOldPPG(value & 0x0fffffff);
	return true;
}

bool TGRSIDataParser::SetPPGNetworkPacket(uint32_t value, TPPGData* ppgevent)
{
	// Ignores the network packet number (for now)
	if((value & 0xf0000000) != 0xd0000000) {
		return false;
	}
	ppgevent->SetNetworkPacketId(value & 0x00ffffff);

	return true;
}

bool TGRSIDataParser::SetPPGLowTimeStamp(uint32_t value, TPPGData* ppgevent)
{
	// the PPG stores the raw values of low and high timestamp
	// SetTimeStamp caluculates the correct (multiplied by 10) timestamp from these
	// and is called by both SetLowTimeStamp and SetHighTimeStamp
	ppgevent->SetLowTimeStamp(value & 0x0fffffff);
	return true;
}

bool TGRSIDataParser::SetPPGHighTimeStamp(uint32_t value, TPPGData* ppgevent)
{
	// the PPG stores the raw values of low and high timestamp
	// SetTimeStamp caluculates the correct (multiplied by 10) timestamp from these
	// and is called by both SetLowTimeStamp and SetHighTimeStamp
	ppgevent->SetHighTimeStamp(value & 0x0fffffff);
	return true;
}

int TGRSIDataParser::GriffinDataToScalerEvent(uint32_t* data, int address)
{
	auto* scalerEvent = new TScalerData;
	scalerEvent->SetAddress(address);
	int x          = 1; // We have already read the header so we can skip the 0th word.
	int failedWord = -1;

	// we expect a word starting with 0xd containing the network packet id
	// this is a different format than the others because it will not always be in the scaler word
	if(SetScalerNetworkPacket(data[x], scalerEvent)) {
		x++;
	}

	// we expect a word starting with 0xa containing the 28 lowest bits of the timestamp
	if(!SetScalerLowTimeStamp(data[x++], scalerEvent)) {
		TParsingDiagnostics::Get()->BadFragment(-3); // use detector type -3 for scaler data
		fState     = EDataParserState::kBadScalerLowTS;
		failedWord = x;
		throw TGRSIDataParserException(fState, failedWord, false);
	}
	// followed by four scaler words (32 bits each)
	for(int i = 0; i < 4; ++i) {
		if(!SetScalerValue(i, data[x++], scalerEvent)) {
			TParsingDiagnostics::Get()->BadFragment(-3); // use detector type -3 for scaler data
			fState     = EDataParserState::kBadScalerValue;
			failedWord = x;
			throw TGRSIDataParserException(fState, failedWord, false);
		}
	}
	// and finally the trailer word with the highest 24 bits of the timestamp
	int scalerType = 0;
	if(!SetScalerHighTimeStamp(data[x++], scalerEvent, scalerType)) {
		TParsingDiagnostics::Get()->BadFragment(-3); // use detector type -3 for scaler data
		fState     = EDataParserState::kBadScalerHighTS;
		failedWord = x;
		throw TGRSIDataParserException(fState, failedWord, false);
	}

	if(scalerType == 0) { // deadtime scaler
		TDeadtimeScalerQueue::Get()->Add(scalerEvent);
	} else if(scalerType == 1) { // rate scaler
		// the rate scaler has only one real value, the rate
		scalerEvent->ResizeScaler();
		TRateScalerQueue::Get()->Add(scalerEvent);
	} else {                                        // unknown scaler type
		TParsingDiagnostics::Get()->BadFragment(-3); // use detector type -3 for scaler data
		fState     = EDataParserState::kBadScalerType;
		failedWord = x;
		throw TGRSIDataParserException(fState, failedWord, false);
	}

	TParsingDiagnostics::Get()->GoodFragment(-3); // use detector type -3 for scaler data
	return x;
}

bool TGRSIDataParser::SetScalerNetworkPacket(uint32_t value, TScalerData* scalerEvent)
{
	if((value >> 28) != 0xd) {
		return false;
	}
	scalerEvent->SetNetworkPacketId(value & 0x0fffffff);
	return true;
}

bool TGRSIDataParser::SetScalerLowTimeStamp(uint32_t value, TScalerData* scalerEvent)
{
	// the scaler stores the raw values of low and high timestamp
	// GetTimeStamp caluculates the correct (multiplied by 10) timestamp from these
	if((value >> 28) != 0xa) {
		return false;
	}
	scalerEvent->SetLowTimeStamp(value & 0x0fffffff);
	return true;
}

bool TGRSIDataParser::SetScalerHighTimeStamp(uint32_t value, TScalerData* scalerEvent, int& type)
{
	// the scaler stores the raw values of low and high timestamp
	// GetTimeStamp caluculates the correct (multiplied by 10) timestamp from these
	if((value >> 28) != 0xe || (value & 0xff) != (scalerEvent->GetLowTimeStamp() >> 20)) {
		return false;
	}
	scalerEvent->SetHighTimeStamp((value >> 8) & 0x0000ffff);
	type = (value >> 24 & 0xf);
	return true;
}

bool TGRSIDataParser::SetScalerValue(int index, uint32_t value, TScalerData* scalerEvent)
{
	scalerEvent->SetScaler(index, value);
	return true;
}

int TGRSIDataParser::CaenPsdToFragment(uint32_t* data, int size, std::shared_ptr<TMidasEvent>& event)
{
	/// Converts a Caen flavoured MIDAS events into TFragments and returns the number of events processed
	std::shared_ptr<TFragment> eventFrag = std::make_shared<TFragment>();
	int w = 0;
	int nofFragments = 0;

	if(Options() == nullptr) {
		Options(TGRSIOptions::Get());
	}

	for(int board = 0; w < size; ++board) {
		// read board aggregate header
		if(data[w]>>28 != 0xa) {
			if(data[w] == 0x0) {
				while(w < size) {
					if(data[w++] != 0x0) {
						if(!Options()->SuppressErrors()) {
							std::cerr<<board<<". board - failed on first word, found empty word, but not all following words were empty: "<<w-1<<" 0x"<<std::hex<<std::setw(8)<<std::setfill('0')<<data[w-1]<<std::dec<<std::setfill(' ')<<std::endl;
						}
						return -w;
					}
				}
				return nofFragments;
			}
			if(!Options()->SuppressErrors()) {
				std::cerr<<board<<". board - failed on first word 0x"<<std::hex<<std::setw(8)<<std::setfill('0')<<data[w]<<std::dec<<std::setfill(' ')<<", highest nibble should have been 0xa!"<<std::endl;
			}
			return -w;
		}
		int32_t numWordsBoard = data[w++]&0xfffffff; // this is the number of 32-bit words from this board
		if(w - 1 + numWordsBoard > size) { 
			if(!Options()->SuppressErrors()) {
				std::cerr<<"0 - Missing words, at word "<<w-1<<", expecting "<<numWordsBoard<<" more words for board "<<board<<" (bank size "<<size<<")"<<std::endl;
			}
			return -w;
		}
		uint8_t boardId = data[w]>>27; // GEO address of board (can be set via register 0xef08 for VME)
		//uint16_t pattern = (data[w]>>8) & 0x7fff; // value read from LVDS I/O (VME only)
		uint8_t channelMask = data[w++]&0xff; // which channels are in this board aggregate
		++w;//uint32_t boardCounter = data[w++]&0x7fffff; // ??? "counts the board aggregate"
		uint32_t boardTime = data[w++]; // time of creation of aggregate (does not correspond to a physical quantity)
		//if(boardCounter < gBoardCounter) {
		//	std::cerr<<"current board counter "<<boardCounter<<" is less than previous one "<<gBoardCounter<<", skipping this data"<<std::endl;
		//	return nofFragments;
		//}
		//gBoardCounter = boardCounter;

		for(uint8_t channel = 0; channel < 16; channel += 2) {
			if(((channelMask>>(channel/2)) & 0x1) == 0x0) {
				continue;
			}
			// read channel aggregate header
			if(data[w]>>31 != 0x1) {
				if(!Options()->SuppressErrors()) {
					std::cerr<<"Failed on first word 0x"<<std::hex<<std::setw(8)<<std::setfill('0')<<data[w]<<std::dec<<std::setfill(' ')<<", highest bit should have been set!"<<std::endl;
				}
				return -w;
			}
			int32_t numWords = data[w++]&0x3fffff;//per channel
			if(w >= size) {
				if(!Options()->SuppressErrors()) {
					std::cerr<<"1 - Missing words, got only "<<w-1<<" words for channel "<<channel<<" (bank size "<<size<<")"<<std::endl;
				}
				return -w;
			}
			if(((data[w]>>29) & 0x3) != 0x3) {
				if(!Options()->SuppressErrors()) {
					std::cerr<<"Failed on second word 0x"<<std::hex<<std::setw(8)<<std::setfill('0')<<data[w]<<std::dec<<std::setfill(' ')<<", bits 29 and 30 should have been set!"<<std::endl;
				}
				return -w;
			}
			bool dualTrace = ((data[w]>>31) == 0x1);
			bool extras    = (((data[w]>>28) & 0x1) == 0x1);
			bool waveform  = (((data[w]>>27) & 0x1) == 0x1);
			uint8_t extraFormat = ((data[w]>>24) & 0x7);
			//for now we ignore the information which traces are stored:
			//bits 22,23: if(dualTrace) 00 = "Input and baseline", 01 = "CFD and Baseline", 10 = "Input and CFD"
			//            else          00 = "Input", 01 = "CFD"
			//bits 19,20,21: 000 = "Long gate",  001 = "over thres.", 010 = "shaped TRG", 011 = "TRG Val. Accept. Win.", 100 = "Pile Up", 101 = "Coincidence", 110 = reserved, 111 = "Trigger"
			//bits 16,17,18: 000 = "Short gate", 001 = "over thres.", 010 = "TRG valid.", 011 = "TRG HoldOff",           100 = "Pile Up", 101 = "Coincidence", 110 = reserved, 111 = "Trigger"
			int numSampleWords = 4*(data[w++]&0xffff);// this is actually the number of samples divided by eight, 2 sample per word => 4*
			if(w >= size) {
				if(!Options()->SuppressErrors()) {
					std::cerr<<"2 - Missing words, got only "<<w-1<<" words for channel "<<channel<<" (bank size "<<size<<")"<<std::endl;
				}
				return -w;
			}
			int eventSize = numSampleWords+2; // +2 = trigger time words and charge word
			if(extras) ++eventSize;
			if(numWords%eventSize != 2 && !(eventSize == 2 && numWords%eventSize == 0)) { // 2 header words plus n*eventSize should make up one channel aggregate
				if(!Options()->SuppressErrors()) {
					std::cerr<<numWords<<" words in channel aggregate, event size is "<<eventSize<<" => "<<static_cast<double>(numWords-2.)/static_cast<double>(eventSize)<<" events?"<<std::endl;
				}
				return -w;
			}

			// read channel data
			for(int ev = 0; ev < (numWords-2)/eventSize; ++ev) { // -2 = 2 header words for channel aggregate
				eventFrag->SetDaqTimeStamp(boardTime);
				eventFrag->SetAddress(0x8000 + (boardId * 0x100) + channel + (data[w]>>31)); // highest bit indicates odd channel
				if(eventFrag->GetAddress() == 0x8000) eventFrag->SetDetectorType(9); //ZDS will always be in channel 0
				else                                  eventFrag->SetDetectorType(6);
				// these timestamps are in 2ns units
				eventFrag->SetTimeStamp(data[w] & 0x7fffffff);
				++w;
				if(waveform) {
					if(w + numSampleWords >= size) { // need to read at least the sample words plus the charge/extra word
						if(!Options()->SuppressErrors()) {
							std::cerr<<"3 - Missing "<<numSampleWords<<" waveform words, got only "<<w-1<<" words for channel "<<channel<<" (bank size "<<size<<")"<<std::endl;
						}
						return -w;
					}
					for(int s = 0; s < numSampleWords && w < size; ++s, ++w) {
						//eventFrag->AddDigitalWaveformSample(0, (data[w]>>14)&0x1);
						//eventFrag->AddDigitalWaveformSample(1, (data[w]>>15)&0x1);
						if(dualTrace) {
							// all even samples are from the first trace, all odd ones from the second trace
							//eventFrag->AddWaveformSample(data[w]&0x3fff);
							//eventFrag->AddWaveformSample((data[w]>>16)&0x3fff);
							eventFrag->AddWaveformSample(data[w]&0xffff);
							eventFrag->AddWaveformSample((data[w]>>16)&0xffff);
						} else {
							// both samples are from the first trace
							//eventFrag->AddWaveformSample(data[w]&0x3fff);
							//eventFrag->AddWaveformSample((data[w]>>16)&0x3fff);
							eventFrag->AddWaveformSample(data[w]&0xffff);
							eventFrag->AddWaveformSample((data[w]>>16)&0xffff);
						}
						//eventFrag->AddDigitalWaveformSample(0, (data[w]>>30)&0x1);
						//eventFrag->AddDigitalWaveformSample(1, (data[w]>>31)&0x1);
					}
				} else {
					if(w >= size) { // need to read at least the sample words plus the charge/extra word
						if(!Options()->SuppressErrors()) {
							std::cerr<<"3 - Missing words, got only "<<w-1<<" words for channel "<<channel<<" (bank size "<<size<<")"<<std::endl;
						}
						return -w;
					}
				}
				if(extras) {
					//std::cout<<eventFrag->GetAddress()<<" - extra word format "<<static_cast<int>(extraFormat)<<": 0x"<<std::hex<<data[w]<<std::dec<<std::endl;
					switch(extraFormat) {
						case 0: // [31:16] extended time stamp, [15:0] baseline*4
							//eventFrag->Baseline(data[w]&0xffff);
							eventFrag->SetTimeStamp(eventFrag->GetTimeStamp() | static_cast<uint64_t>(data[w]&0xffff0000)<<15);
							break;
						case 1: // [31:16] extended time stamp, 15 trigger lost, 14 over range, 13 1024 triggers, 12 n lost triggers
							eventFrag->SetNetworkPacketNumber((data[w]>>12)&0xf);
							//eventFrag->NLostCount(((data[w]>>12)&0x1) == 0x1);
							//eventFrag->KiloCount(((data[w]>>13)&0x1) == 0x1);
							//eventFrag->OverRange(((data[w]>>14)&0x1) == 0x1);
							//eventFrag->LostTrigger(((data[w]>>15)&0x1) == 0x1);
							eventFrag->SetTimeStamp(eventFrag->GetTimeStamp() | static_cast<uint64_t>(data[w]&0xffff0000)<<15);
							break;
						case 2: // [31:16] extended time stamp,  15 trigger lost, 14 over range, 13 1024 triggers, 12 n lost triggers, [9:0] fine time stamp
							eventFrag->SetTimeStamp(eventFrag->GetTimeStamp() | static_cast<uint64_t>(data[w]&0xffff0000)<<15);
							eventFrag->SetCfd(data[w]&0x3ff);
							eventFrag->SetNetworkPacketNumber((data[w]>>12)&0xf);
							//eventFrag->NLostCount(((data[w]>>12)&0x1) == 0x1);
							//eventFrag->KiloCount(((data[w]>>13)&0x1) == 0x1);
							//eventFrag->OverRange(((data[w]>>14)&0x1) == 0x1);
							//eventFrag->LostTrigger(((data[w]>>15)&0x1) == 0x1);
							break;
						case 4: // [31:16] lost trigger counter, [15:0] total trigger counter
							eventFrag->SetAcceptedChannelId(data[w]&0xffff); // this is actually the lost trigger counter!
							eventFrag->SetChannelId(data[w]>>16);
							break;
						case 5: // [31:16] CFD sample after zero cross., [15:0] CFD sample before zero cross.
							//eventFrag->CfdAfterZC(data[w]&0xffff);
							//eventFrag->CfdBeforeZC(data[w]>>16);
							w++;
							break;
						case 7: // fixed value of 0x12345678
							if(data[w] != 0x12345678) {
								if(!Options()->SuppressErrors()) {
									std::cerr<<"Failed to get debug data word 0x12345678, got "<<std::hex<<std::setw(8)<<std::setfill('0')<<data[w]<<std::dec<<std::setfill(' ')<<std::endl;
								}
								break;
							}
							break;
						default:
							break;
					}
					++w;
				}
				eventFrag->SetCcShort(data[w]&0x7fff);
				eventFrag->SetCcLong((data[w]>>15) & 0x1);//this is actually the over-range bit!
				eventFrag->SetCharge(static_cast<Int_t>(data[w++]>>16));
				Push(GoodOutputQueues(), std::make_shared<TFragment>(*eventFrag));
				eventFrag->Clear();
				++nofFragments;
				event->IncrementGoodFrags();
			} // while(w < size)
		} // for(uint8_t channel = 0; channel < 16; channel += 2)
	} // for(int board = 0; w < size; ++board)

	return nofFragments;
}

int TGRSIDataParser::CaenPhaToFragment(uint32_t* data, int size, std::shared_ptr<TMidasEvent>& event)
{
	/// Converts a Caen flavoured MIDAS events into TFragments and returns the number of events processed
	std::shared_ptr<TFragment> eventFrag = std::make_shared<TFragment>();
	int w = 0;
	int nofFragments = 0;

	if(Options() == nullptr) {
		Options(TGRSIOptions::Get());
	}

	for(int board = 0; w < size; ++board) {
		// read board aggregate header
		if(data[w]>>28 != 0xa) {
			if(data[w] == 0x0) {
				while(w < size) {
					if(data[w++] != 0x0) {
						if(!Options()->SuppressErrors()) {
							std::cerr<<board<<". board - failed on first word, found empty word, but not all following words were empty: "<<w-1<<" 0x"<<std::hex<<std::setw(8)<<std::setfill('0')<<data[w-1]<<std::dec<<std::setfill(' ')<<std::endl;
						}
						return -w;
					}
				}
				return nofFragments;
			}
			if(!Options()->SuppressErrors()) {
				std::cerr<<board<<". board - failed on first word 0x"<<std::hex<<std::setw(8)<<std::setfill('0')<<data[w]<<std::dec<<std::setfill(' ')<<", highest nibble should have been 0xa!"<<std::endl;
			}
			return -w;
		}
		int32_t numWordsBoard = data[w++]&0xfffffff; // this is the number of 32-bit words from this board
		if(w - 1 + numWordsBoard > size) { 
			if(!Options()->SuppressErrors()) {
				std::cerr<<"0 - Missing words, at word "<<w-1<<", expecting "<<numWordsBoard<<" more words for board "<<board<<" (bank size "<<size<<")"<<std::endl;
			}
			return -w;
		}
		uint8_t boardId = data[w]>>27; // GEO address of board (can be set via register 0xef08 for VME)
		//bool failFlag = (data[w]>>26) & 0x1; // board fail flag (PLL lock lost or overheating)
		//uint16_t pattern = (data[w]>>8) & 0x7fff; // value read from LVDS I/O (VME only)
		uint8_t channelMask = data[w++]&0xff; // which channels are in this board aggregate
		++w;//uint32_t boardCounter = data[w++]&0x7fffff; // ??? "counts the board aggregate"
		uint32_t boardTime = data[w++]; // time of creation of aggregate (does not correspond to a physical quantity)
		//if(boardCounter < gBoardCounter) {
		//	std::cerr<<"current board counter "<<boardCounter<<" is less than previous one "<<gBoardCounter<<", skipping this data"<<std::endl;
		//	return nofFragments;
		//}
		//gBoardCounter = boardCounter;

		for(uint8_t channel = 0; channel < 8; ++channel) {
			// check if the bit for this channel is set in the channel mask
			if(((channelMask>>channel) & 0x1) == 0x0) {
				continue;
			}
			// read channel aggregate header
			if(data[w]>>31 != 0x1) {
				if(!Options()->SuppressErrors()) {
					std::cerr<<"Failed on first word 0x"<<std::hex<<std::setw(8)<<std::setfill('0')<<data[w]<<std::dec<<std::setfill(' ')<<", highest bit should have been set (to get format info)!"<<std::endl;
				}
				return -w;
			}
			int32_t numWords = data[w++]&0x7fffffff;//per channel
			if(w >= size) {
				if(!Options()->SuppressErrors()) {
					std::cerr<<"1 - Missing words, got only "<<w-1<<" words for channel "<<channel<<" (bank size "<<size<<")"<<std::endl;
				}
				return -w;
			}
			//bool dualTrace      = ((data[w]>>31) == 0x1);
			//bool energyEnabled  = ((data[w]>>30) == 0x1);
			//bool timeEnabled    = ((data[w]>>29) == 0x1);
			bool extras         = (((data[w]>>28) & 0x1) == 0x1);
			bool waveform       = (((data[w]>>27) & 0x1) == 0x1);
			uint8_t extraFormat = ((data[w]>>24) & 0x7);
			//for now we ignore the information which traces are stored:
			//bits 22,23: 00 = "Input", 01 = "RC-CR - first step", 10 = "RC-CR2 - second step", 11 = "Trapezoid"
			//bits 20,21: 00 = "Input", 01 = "Threshold - of RC-CR2", 10 = "Trapezoid minus baseline", 11 = "baseline of trapezoid"
			//bits 16-19: 0000 = "Peaking" - aka when energy is evaluated,
			//            0001 = "Armed" - when RC-CR2 crosses threshold,
			//            0010 = "Peak Run" - starts with trigger until end of event,
			//            0011 = "Pile-Up" - time interval when energy calc. is disabled due to pile-up,
			//            0100 = "Peaking" - aka when energy is evaluated (again?),
			//            0101 = "Trg Validation Window" - trigger validation acceptance window TVAW,
			//            0110 = "BSL Freeze" - shows when baseline is frozen for energy evaluation,
			//            0111 = "Trg Holdoff" - shows trigger holdoff parameter,
			//            1000 = "Trg Validation" - shows trigger validation signal TRG_VAL,
			//            1001 = "Acq Busy" - 1 if board is busy or vetoed,
			//            1010 = "Trg Window" - not used,
			//            1011 = "Ext Trg" - shows external trigger when available,
			//            1100 = "Busy" - shows when memory board is full
			int numSampleWords = 4*(data[w++]&0xffff);// this is actually the number of samples divided by eight, 2 sample per word => 4*
			if(w >= size) {
				if(!Options()->SuppressErrors()) {
					std::cerr<<"2 - Missing words, got only "<<w-1<<" words for channel "<<channel<<" (bank size "<<size<<")"<<std::endl;
				}
				return -w;
			}
			int eventSize = numSampleWords+2; // +2 = trigger time words and charge word
			if(extras) ++eventSize;
			if(numWords%eventSize != 2 && !(eventSize == 2 && numWords%eventSize == 0)) { // 2 header words plus n*eventSize should make up one channel aggregate
				if(!Options()->SuppressErrors()) {
					std::cerr<<numWords<<" words in channel aggregate, event size is "<<eventSize<<" ("<<numWords%eventSize<<") => "<<static_cast<double>(numWords-2.)/static_cast<double>(eventSize)<<" events?"<<std::endl;
				}
				return -w;
			}

			// read channel data
			for(int ev = 0; ev < (numWords-2)/eventSize; ++ev) { // -2 = 2 header words for channel aggregate
				eventFrag->SetDaqTimeStamp(boardTime);
				eventFrag->SetAddress(0x8000 + (boardId * 0x100) + channel + (data[w]>>31)); // highest bit indicates odd channel
				eventFrag->SetDetectorType(0); // PHA firmware only used for HPGe?
				// these timestamps are in 2ns units
				eventFrag->SetTimeStamp(data[w++] & 0x7fffffff);
				if(waveform) {
					if(w + numSampleWords >= size) { // need to read at least the sample words plus the charge/extra word
						if(!Options()->SuppressErrors()) {
							std::cerr<<"3 - Missing "<<numSampleWords<<" waveform words, got only "<<w-1<<" words for channel "<<channel<<" (bank size "<<size<<")"<<std::endl;
						}
						return -w;
					}
					for(int s = 0; s < numSampleWords && w < size; ++s, ++w) {
						// we don't care if we use dual trace and which bits are what (analog of digital waveform)
						// everything gets put into the normale waveform and will have to be separated during analysis
						// higher bits are later sample
						eventFrag->AddWaveformSample(data[w]&0xffff);
						eventFrag->AddWaveformSample((data[w]>>16)&0xffff);
					}
				} else {
					if(w >= size) { // need to read at least the sample words plus the charge/extra word
						if(!Options()->SuppressErrors()) {
							std::cerr<<"3 - Missing words, got only "<<w-1<<" words for channel "<<channel<<" (bank size "<<size<<")"<<std::endl;
						}
						return -w;
					}
				}
				if(extras) {
					//std::cout<<eventFrag->GetAddress()<<" - extra word format "<<static_cast<int>(extraFormat)<<": 0x"<<std::hex<<data[w]<<std::dec<<std::endl;
					switch(extraFormat) {
						case 0: // [31:16] extended time stamp, [15:0] trapezoid baseline*4
							//eventFrag->Baseline(data[w]&0xffff);
							eventFrag->SetTimeStamp(eventFrag->GetTimeStamp() | static_cast<uint64_t>(data[w]&0xffff0000)<<15);
							break;
						case 2: // [31:16] extended time stamp, [15:0] fine time stamp (linear int. of RC-CR2 before and after zero-crossing)
							eventFrag->SetTimeStamp(eventFrag->GetTimeStamp() | static_cast<uint64_t>(data[w]&0xffff0000)<<15);
							eventFrag->SetCfd(data[w]&0xffff);
							break;
						case 4: // [31:16] lost trigger counter, [15:0] total trigger counter
							eventFrag->SetAcceptedChannelId(data[w]&0xffff); // this is actually the lost trigger counter!
							eventFrag->SetChannelId(data[w]>>16);
							break;
						case 5: // [31:16] sample before zero cross., [15:0] sample after zero cross.
							//eventFrag->CfdBeforeZC(data[w]&0xffff);
							//eventFrag->CfdAfterZC(data[w]>>16);
							++w;
							break;
						case 1: // reserved
						case 3: // reserved
						case 7: // reserved
							break;
						default:
							break;
					}
					++w;
				}
				// highest bit is actually the pile-up or roll-over bit!
				eventFrag->SetCharge(static_cast<Int_t>(data[w]&0xffff));
				// extras [20:16] or [23:16]?
				// 0 - lost event due to full memory board
				// 1 - roll over of time stamp (set by bit[26] of 0x1n80 to create fake event with this bit set)
				// 2 - reserved
				// 3 - fake event - from time stamp roll-over
				// 4 - input saturation
				// 5 - lost trigger - every n lost events this flag is high (n from bits[17:16] of 0x1na0
				// 6 - total trigger - every n total events this flag is high (n from bits[17:16] of 0x1na0
				// 7 - match coincidence - if bit[19] of 0x1na0 set then all events matching coincidence criteria are saved with this bit
				// 8 - no match coincidence - if bit[19] of 0x1na0 set then all events NOT matching coincidence criteria are saved with this bit
				++w;
				Push(GoodOutputQueues(), std::make_shared<TFragment>(*eventFrag));
				eventFrag->Clear();
				++nofFragments;
				event->IncrementGoodFrags();
			} // while(w < size)
		} // for(uint8_t channel = 0; channel < 16; channel += 2)
	} // for(int board = 0; w < size; ++board)

	return nofFragments;
}

/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////

int TGRSIDataParser::EPIXToScalar(float* data, int size, unsigned int midasSerialNumber, time_t midasTime)
{
	int                         NumFragsFound = 1;
	std::shared_ptr<TEpicsFrag> EXfrag        = std::make_shared<TEpicsFrag>();

	EXfrag->DaqTimeStamp(midasTime);
	EXfrag->DaqId(midasSerialNumber);

	for(int x = 0; x < size; x++) {
		EXfrag->AddData(data[x]);
		EXfrag->AddName(TEpicsFrag::GetEpicsVariableName(x));
	}

	ScalerOutputQueue()->Push(EXfrag);
	return NumFragsFound;
}

/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////**************************EMMA Stuff***************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////

static Long64_t xferhfts;     // Time stamp to be transferred from ADC to TDC; 10 ns ticks
static time_t   xfermidts;    // Midas time stamp of events, hopefully the same
unsigned int    xfermidsn;    // Midas serial number


int TGRSIDataParser::EmmaMadcDataToFragment(uint32_t* data, int size, std::shared_ptr<TMidasEvent>& event)
{
	/// Converts a MIDAS File from the Emma DAQ into a TFragment.
	int numFragsFound = 0;
	std::shared_ptr<TFragment> eventFrag     = std::make_shared<TFragment>();
	xfermidts = event->GetTimeStamp();// to check against EMMT bank
	eventFrag->SetDaqTimeStamp(xfermidts);
	xfermidsn = event->GetSerialNumber(); // to chck againts EMMT bank
	eventFrag->SetDaqId(xfermidsn);
	eventFrag->SetDetectorType(12);

	int      x     = 0;
	uint32_t dword = *(data + x);

	uint32_t type;
	uint32_t adcchannel;
	uint32_t adcdata;
	uint32_t adctimestamp;
	uint32_t adchightimestamp;

	eventFrag->SetTimeStamp(0);
	for(x=size; x-- > 0; ) { // Reads the event backwards 
		dword = *(data + x);
		type  = (dword & 0xf0000000) >> 28;
		switch(type) {
			case 0x0: // Reads 	the charge and channel number
				{
					if((dword & 0x00800000) != 0) {
						adchightimestamp=dword&0x0000ffff;
						eventFrag->AppendTimeStamp((static_cast<Long64_t>(adchightimestamp))*static_cast<Long64_t>(0x0000000040000000)); // This should shift the time stamp 30 bits
						xferhfts = eventFrag->GetTimeStamp();
					} else if ((dword & 0x04000000) != 0) { // GH verify that this is a good ADC reading
						adcchannel = (dword>>16)&0x1F; // ADC Channel Number
						adcdata = (dword & 0xfff); // ADC Charge
						std::shared_ptr<TFragment> transferfrag = std::make_shared<TFragment>(*eventFrag);
						transferfrag->SetCharge(static_cast<Int_t>(adcdata));
						transferfrag->SetAddress(0x800000 + adcchannel);
						TChannel* chan = TChannel::GetChannel(transferfrag->GetAddress(), false);
						if(chan == nullptr) {
							chan = Channel();
						}
						Push(GoodOutputQueues(), transferfrag);
						numFragsFound++;
					}
				} 
				break;

			case 0x4: // First DWORD in event, read last and writes the fragment
				//            numFragsFound++;
				eventFrag = nullptr;
				return numFragsFound;
				break;

			case 0xe:
			case 0xf:
			case 0xd:
			case 0xc: // Last 30 bits of timestamp
				adctimestamp =(dword&0x3FFFFFFF);
				eventFrag->AppendTimeStamp(static_cast<Long64_t>(adctimestamp));
				break;
			default: break;
		} // end swich
	} // end for read backwards

	return numFragsFound;
}

// kludge by GH
static uint32_t wraparoundcounter = 0xffffffff; // Needed for bad data at start of run before GRIFFIN starts
static uint32_t lasttimestamp;     // "last" time stamp for simple wraparound algorightm
static uint32_t countsbetweenwraps; // number of counts between wraparounds

int TGRSIDataParser::EmmaTdcDataToFragment(uint32_t* data, int size, std::shared_ptr<TMidasEvent>& event)
{
	/// Building TDC events, duplicating logic from EmmaMadcDataToFragment
	int numFragsFound = 0;
	std::shared_ptr<TFragment> eventFrag     = std::make_shared<TFragment>();
	eventFrag->SetDaqTimeStamp(event->GetTimeStamp());
	eventFrag->SetDaqId(event->GetSerialNumber());
	eventFrag->SetDetectorType(13);

	int      x     = 0;
	int failedWord = -1; // Variable stores which word we failed on (if we fail). This is somewhat duplicate information
	// to fState, but makes things easier to track.
	bool multipleErrors = false; // Variable to store if multiple errors occured parsing one fragment
	uint32_t tmpTimestamp = 0;
	uint32_t tmpAddress = 0;
	Long64_t ts;

	std::vector<uint32_t> addresses;
	std::vector<uint32_t> charges;

	// we simply loop through the data and read what we get. No checks are made that every word we want is present.
	for(x=0; x < size; ++x) {
		switch((data[x] >> 27) & 0x1F) {
			case 0x8: //global header
				eventFrag->SetModuleType(data[x] & 0x1f); // GEO
				eventFrag->SetAcceptedChannelId((data[x] >> 5) & 0x3fffff); // event count
				break;
			case 0x1: //TDC header
				tmpAddress = (data[x] >> 16) & 0x300;//16 = 24 - 8
				eventFrag->SetChannelId((data[x] >> 12) & 0xfff); // event id
				eventFrag->SetNetworkPacketNumber(data[x] & 0xfff); // bunch id
				break;
			case 0x0: // TDC measurement
				// we can get multiple of these per event, but we need the subsequent information from the trailer etc.
				// so we store the words for now and build fragments at the very end
				//
				// we use the trailing/leading bit as additional address information
				// VB's original code: addresses.push_back(((data[x] >> 18) & 0x800) | tmpAddress | ((data[x] >> 19) & 0x7f));//15 = 26 - 11
				addresses.push_back(0x900000 + ((data[x] >> 19) & 0xff ) );  // address = 0x00900000 + channel + 0x80 for a trailing measurement
				charges.push_back(data[x] & 0x7ffff);
				break;
			case 0x3: // TDC trailer
				if((tmpAddress != ((data[x] >> 16) & 0x300)) || (eventFrag->GetChannelId() != ((data[x] >> 12) & 0xfff))) {
					// either the trailer tdc doesn't match the header tdc, or the trailer event id doesn't match the header event id
					TParsingDiagnostics::Get()->BadFragment(13); // hard-coded 13 for TDC for now
					if(fState == EDataParserState::kGood) {
						fState     = EDataParserState::kBadFooter;
						failedWord = x;
					} else {
						multipleErrors = true;
					}
					Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
				}
				eventFrag->SetNumberOfPileups(data[x] & 0xfff);
				break;
			case 0x4: // TDC error
				eventFrag->SetAddress((data[x] >> 16) & 0x300);//16 = 24 - 8
				eventFrag->SetCharge(static_cast<Int_t>((data[x]) & 0xFFF)); // error flags
				TParsingDiagnostics::Get()->BadFragment(13); // hard-coded 13 for TDC for now
				if(fState == EDataParserState::kGood) {
					fState     = EDataParserState::kFault;
					failedWord = x;
				} else {
					multipleErrors = true;
				}
				Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
				break;
			case 0x11: // extended trigger time
				tmpTimestamp = (data[x] & 0x7FFFFFF) << 5;
				break;
			case 0x10: // trailer
				//(data[x] >> 26) & 1 - trigger lost
				//(data[x] >> 25) & 1 - output buffer overflow
				//(data[x] >> 24) & 1 - tdc error
				//(data[x] >> 5) & 0xFFFF - word count
				tmpTimestamp= tmpTimestamp | ((data[x]) & 0x1f); // GEO bits of trailer include additional 5 bits of extended trigger time
				// GH Handle wraparound
				if (tmpTimestamp < lasttimestamp) { // Assume this means you wrapped around
					wraparoundcounter++; // How many times it wrapped around
					countsbetweenwraps=0; // Reset wraparound counter
				}
				lasttimestamp=tmpTimestamp; // so far so good
				ts = static_cast<Long64_t>(lasttimestamp); // start with 32 bits
				ts += static_cast<Long64_t>(0x100000000)*static_cast<Long64_t>(wraparoundcounter); // add wrap around
				ts = ts * static_cast<Long64_t>(5); // multiply by 5
				ts = ts >> 1; // divide by 2
				// And now that we've actually done all this:
				// Check if there's a somewhat valid timestamp from an ADC
				// the && 0 at the end suppresses the xfer
				if ( (event->GetSerialNumber()==xfermidsn) && (event->GetTimeStamp()==xfermidts)) { // Valid data from prior MADC bank
					eventFrag->SetTimeStamp(xferhfts);
				} else {
					eventFrag->SetTimeStamp(ts);
				}
				// got all the data, so now we can loop over the hits and write them
				if(addresses.size() != charges.size()) {
					std::cout<<"Something went horribly wrong, the number of addresses read "<<addresses.size()<<" doesn't match the number of charges read "<<charges.size()<<std::endl;
					return 0;
				}
				for(size_t i = 0; i < addresses.size(); ++i) {
					size_t duped = 0;
					size_t ii;
					if (i>0) {
						for (ii=0; ii<i; ii++) {
							if (addresses[i]==addresses[ii]) {
								duped++;
							}
						}
					}
					if (duped==0) {
						eventFrag->SetAddress(addresses[i]);
						eventFrag->SetCharge(static_cast<Int_t>(charges[i]));
						Push(GoodOutputQueues(), std::make_shared<TFragment>(*eventFrag));
						++numFragsFound;
					} 
				}

				// clear the old data
				addresses.clear();
				charges.clear();
				tmpTimestamp = 0;
				tmpAddress = 0;
				break;
			default:
				TParsingDiagnostics::Get()->BadFragment(13); // hard-coded 13 for TDC for now
				if(fState == EDataParserState::kGood) {
					fState     = EDataParserState::kUndefined;
					failedWord = x;
				} else {
					multipleErrors = true;
				}
				Push(*BadOutputQueue(), std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
				break;

		}
	}
	return numFragsFound;
}

int TGRSIDataParser::EmmaRawDataToFragment(uint32_t* data, int size, std::shared_ptr<TMidasEvent>&)
{
	/// Extract SRAW data, i.e. the instantaneous rates
	auto* scaler = new TScalerData;
	scaler->SetAddress(0xfffe); // magic address for raw scaler
	// counter used to set time
	static UInt_t nofRawScalers = 0;
	scaler->SetLowTimeStamp(nofRawScalers++);
	scaler->SetScaler(data, size);
	TRateScalerQueue::Get()->Add(scaler);
	return 0;
}

int TGRSIDataParser::EmmaSumDataToFragment(uint32_t* data, int size, std::shared_ptr<TMidasEvent>&)
{
	/// Extract SSUM data, i.e. the cumulative counts
	auto* scaler = new TScalerData;
	scaler->SetAddress(0xffff); // magic address for sum scaler
	// counter used to set time
	static UInt_t nofSumScalers = 0;
	scaler->SetLowTimeStamp(nofSumScalers++);
	scaler->SetScaler(data, size);
	TRateScalerQueue::Get()->Add(scaler);
	return 0;
}

--
-- Copyright (C) 2016-2017 Lennart Andersson.
--
-- This file is part of OPS (Open Publish Subscribe).
--
-- OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
-- it under the terms of the GNU Lesser General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Lesser General Public License for more details.
--
-- You should have received a copy of the GNU Lesser General Public License
-- along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.

with Ops_Pa.Transport_Pa.Receiver_Pa.MulticastReceiver_Pa;

use Ops_Pa.Transport_Pa.Receiver_Pa.MulticastReceiver_Pa;

package body Ops_Pa.Transport_Pa.Receiver_Pa is

  use type Com_Signal_Pa.Event_T;

  function ErrorService( Self : Receiver_Class ) return ErrorService_Class_At is
  begin
    return Self.ErrorService;
  end;

  procedure SetErrorService( Self : in out Receiver_Class; es : ErrorService_Class_At ) is
  begin
    Self.ErrorService := es;
  end;

  function LastErrorCode( Self : Receiver_Class ) return Integer is
  begin
    return Self.LastErrorCode;
  end;

  procedure InitInstance( Self : in out Receiver_Class; SelfAt : Receiver_Class_At ) is
  begin
    Self.DataNotifier := ReceiveNotifier_Pa.Create( Ops_Class_At(SelfAt) );
    Self.Receiver_Pr.Start;
  end;

  procedure Finalize( Self : in out Receiver_Class ) is
  begin
    Self.TerminateFlag := True;
    Self.EventsToTask.Signal(TerminateEvent_C);
    Self.Receiver_Pr.Finish;
    ReceiveNotifier_Pa.Free(Self.DataNotifier);
  end;

  procedure addListener( Self : in out Receiver_Class; Client : ReceiveNotifier_Pa.Listener_Interface_At ) is
  begin
    Self.DataNotifier.addListener( Client );
  end;

  procedure removeListener( Self : in out Receiver_Class; Client : ReceiveNotifier_Pa.Listener_Interface_At ) is
  begin
    Self.DataNotifier.removeListener( Client );
  end;

  task body Receiver_Pr_T is
    Events : Com_Signal_Pa.Event_T;
  begin
    accept Start;
    while not Self.TerminateFlag loop
      begin
        Self.EventsToTask.WaitForAny(Events);
        exit when (Events and TerminateEvent_C) /= Com_Signal_Pa.NoEvent_C;
        if (Events and StartEvent_C) /= Com_Signal_Pa.NoEvent_C then
          Self.Run;
        end if;
      exception
        when others =>
          Self.ErrorService.Report( "Receiver", "Receiver_Pr", "Got exception from Receiver.Run()" );
      end;
    end loop;
    accept Finish;
  end Receiver_Pr_T;

  procedure Run( Self : in out Receiver_Class ) is
  begin
    null;
  end;

  -- --------------------------------------------------------------------------

  function getReceiver(top : Topic_Class_At; dom : Domain_Class_At; Reporter : ErrorService_Class_At) return Receiver_Class_At is
    Result : Receiver_Class_At := null;
    localif : String := doSubnetTranslation(top.LocalInterface);
  begin
    if top.Transport = TRANSPORT_MC then
      Result := Receiver_Class_At(Ops_Pa.Transport_Pa.Receiver_Pa.MulticastReceiver_Pa.
        Create( string(top.DomainAddress),
                Integer(top.Port),
                localIf,
                top.InSocketBufferSize));

    elsif top.Transport = TRANSPORT_TCP then
      raise Not_Yet_Implemented;
      return null;
      --Result := TTCPClientReceiver.Create(string(top.DomainAddress), top.Port, top.InSocketBufferSize);

    elsif top.Transport = TRANSPORT_UDP then
      raise Not_Yet_Implemented;
      return null;
      --Result := TUDPReceiver.Create(0, localIf, top.InSocketBufferSize);
    end if;

    if Result /= null then
      Result.SetErrorService( Reporter );
    end if;

    return Result;
  end;

end Ops_Pa.Transport_Pa.Receiver_Pa;

/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2016-2017 XMRig       <support@xmrig.com>
 *
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include "api/Api.h"
#include "net/SubmitResult.h"
#include "proxy/events/AcceptEvent.h"
#include "proxy/Stats.h"


Stats::Stats()
{
    m_data.startTime = uv_now(uv_default_loop());
}


Stats::~Stats()
{
}


void Stats::tick(uint64_t ticks)
{
    ticks++;

#   ifndef XMRIG_NO_API
    Api::tick(m_data);
#   endif
}


void Stats::onEvent(IEvent *event)
{
    switch (event->type())
    {
    case IEvent::ConnectionType:
        m_data.connections++;
        m_data.miners++;

        if (m_data.miners > m_data.maxMiners) {
            m_data.maxMiners = m_data.miners;
        }
        break;

    case IEvent::CloseType:
        m_data.connections--;
        m_data.miners--;
        break;

    case IEvent::AcceptType:
        add(static_cast<AcceptEvent*>(event)->result);
        break;

    default:
        break;
    }
}


void Stats::onRejectedEvent(IEvent *event)
{
    switch (event->type())
    {
    case IEvent::SubmitType:
        m_data.invalid++;
        break;

    case IEvent::AcceptType:
        m_data.rejected++;
        break;

    default:
        break;
    }
}


void Stats::add(const SubmitResult &result)
{
    m_data.accepted++;
    m_data.hashes += result.diff;

    const size_t ln = m_data.topDiff.size() - 1;
    if (result.actualDiff > m_data.topDiff[ln]) {
        m_data.topDiff[ln] = result.actualDiff;
        std::sort(m_data.topDiff.rbegin(), m_data.topDiff.rend());
    }

    m_data.latency.push_back(result.elapsed > 0xFFFF ? 0xFFFF : (uint16_t) result.elapsed);
}